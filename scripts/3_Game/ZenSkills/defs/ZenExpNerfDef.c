class ZenActionStreak
{
	int  count;     // how many times in-window
	int  lastTick;  // ms
	
	void ZenActionStreak() 
	{ 
		count = 0; 
		lastTick = 0; 
	}
}

class ZenSkillsExpNerf
{
	vector m_AnchorPos;                     			// bubble anchor
	int m_InBubbleActionCountPrecheck;					// counts every action performed while inside bubble
	int m_InBubbleActionCount;           				// counts every action im bubble AFTER above is reached
	ref map<string, ref ZenActionStreak> m_PerAction; 	// actionKey -> streak

	void ZenSkillsExpNerf()
	{
		m_AnchorPos = vector.Zero;
		m_InBubbleActionCountPrecheck = 0;
		m_InBubbleActionCount = 0;
		m_PerAction = new map<string, ref ZenActionStreak>();
	}

	void ResetAll(vector newAnchor)
	{
		m_AnchorPos = newAnchor;
		m_InBubbleActionCountPrecheck = 0;
		m_InBubbleActionCount = 0;
		m_PerAction.Clear();
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[Nerf] ResetAll: anchor=" + m_AnchorPos);
		#endif
	}

	void EnsureAnchor(vector pos)
	{
		if (m_AnchorPos == vector.Zero)
			ResetAll(pos);
	}

	// Call once per XP event BEFORE computing nerf
	void Update(vector pos, string actionKey)
	{
		int now = GetGame().GetTime();
		EnsureAnchor(pos);

		float dist = vector.Distance(pos, m_AnchorPos);
		if (dist > GetZenSkillsEXP().ExpNerfConfig.NerfBubbleRadiusMeters)
		{
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[Nerf] Moved " + dist + "m > " + GetZenSkillsEXP().ExpNerfConfig.NerfBubbleRadiusMeters + "m > reset nerfs.");
			#endif
			ResetAll(pos);
		}
		else
		{
			if (m_InBubbleActionCountPrecheck < GetZenSkillsEXP().ExpNerfConfig.NerfBubbleMaxActions)
			{
				m_InBubbleActionCountPrecheck++;
				
			}
			else 
			{
				m_InBubbleActionCount++; // universal distance nerf stacks while staying in bubble
			}
		}

		// per-action streak (only nerfs this actionKey)
		ZenActionStreak streak = m_PerAction.Get(actionKey);
		if (!streak)
		{
			streak = new ZenActionStreak();
			m_PerAction.Set(actionKey, streak);
		}
		
		// decay if outside window
		if (now - streak.lastTick > GetZenSkillsEXP().ExpNerfConfig.NerfActionResetTimeMs)
			streak.count = 0;

		streak.count++;
		streak.lastTick = now;

		#ifdef ZENSKILLSDEBUG
		string debugMsg = "[Nerf] Update: action=" + actionKey;
		debugMsg = debugMsg + " streak=" + streak.count;
		debugMsg = debugMsg + " distCnt=" + m_InBubbleActionCount;
		debugMsg = debugMsg + " distFactor=" + GetDistanceFactor();
		debugMsg = debugMsg + " repeatFactor=" + GetActionFactor(actionKey);
		debugMsg = debugMsg + " dist=" + dist;
		ZenSkillsPrint(debugMsg);
		#endif
	}

	float GetDistanceFactor()
	{
		// first action in bubble > count=1 > 5% by default; tweak if you want it to start at 0% (use max(0, count-1))
		int steps = m_InBubbleActionCount;
		float nerf = Math.Clamp(GetZenSkillsEXP().ExpNerfConfig.NerfPercentPerActionInBubble * steps, 0.0, GetZenSkillsEXP().ExpNerfConfig.NerfMaxForBubbleAction);
		return 1.0 - nerf;
	}

	float GetActionFactor(string actionKey)
	{
		ZenActionStreak streak = m_PerAction.Get(actionKey);
		if (!streak) 
			return 1.0;

		// only penalize repeats beyond the first time
		int repeats = Math.Max(0, streak.count - 1);
		float nerf = Math.Clamp(GetZenSkillsEXP().ExpNerfConfig.NerfPercentPerActionRepeat * repeats, 0.0, GetZenSkillsEXP().ExpNerfConfig.NerfMaxForRepeatAction);
		return 1.0 - nerf;
	}

	// Final multiplier = distance (applies to all) * per-action (only this action)
	float GetCombinedFactor(string actionKey)
	{
		float df = GetDistanceFactor();
		float af = GetActionFactor(actionKey);
		return df * af;
	}

	int GetCombinedPercent(string actionKey)
	{
		float f = GetCombinedFactor(actionKey);
		return Math.Round((1.0 - f) * 100.0);
	}
}
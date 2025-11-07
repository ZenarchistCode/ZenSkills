class ZenSkillsExpBoostMdfr: ModifierBase
{
	override void Init()
	{
		m_TrackActivatedTime 	= true;
		//m_AnalyticsStatsEnabled = true;
		m_IsPersistent 			= true;
		m_ID 					= ZenSkillsModifiers.MDF_EXPBOOST;
		m_TickIntervalInactive 	= DEFAULT_TICK_TIME_INACTIVE;
		m_TickIntervalActive 	= 1;
		DisableActivateCheck();
	}

	void ExtendBySeconds(PlayerBase player, int addSeconds)
	{
		// We "extend" by reducing elapsed time, so the remaining time increases.
		float used = GetAttachedTime();
		float newUsed = used - addSeconds;

		SetAttachedTime(newUsed);

		#ifdef ZENSKILLSDEBUG
		string debugMsg = "[ExpBoost] ExtendBySeconds: +" + addSeconds + "s";
		debugMsg = debugMsg + " used=" + used;
		debugMsg = debugMsg + " newUsed=" + newUsed;
		debugMsg = debugMsg + " remaining=" + (GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostTime - newUsed) + "s";
		debugMsg = debugMsg + " - SendRPC!";
		#endif
		
		SendZenSkillsTimerRPC(player);
	}

	override bool ActivateCondition(PlayerBase player) 
	{ 
		return false; 
	}
	
	override void OnReconnect(PlayerBase player)
	{
		OnActivate(player);
	}
	
	override string GetDebugText()
	{
		return (GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostTime - GetAttachedTime()).ToString();
	}
	
	override void OnActivate(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Activate ZenSkillsExpBoostMdfr - send RPC!");
		#endif
		
		SendZenSkillsTimerRPC(player);
	}
	
	override void OnDeactivate(PlayerBase player)
	{		
		if (!player || !player.GetIdentity())
			return;
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Deactivate ZenSkillsExpBoostMdfr - send RPC!");
		#endif
		
		GetGame().RPCSingleParam(player, ZenSkillConstants.RPC_ZenExpBoostNotify, new Param2<bool, int>(false, 0), true, player.GetIdentity());
	}
	
	override bool DeactivateCondition(PlayerBase player)
	{
		return (GetAttachedTime() >= GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostTime);
	}
	
	int ZenGetRemainingSeconds()
	{
		if (!IsActive()) 
			return 0;
		
		float lifetime = GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostTime;
		float remaining = lifetime - GetAttachedTime();
		if (remaining < 0) 
			remaining = 0;
		
		return Math.Round(remaining);
	}
	
	void SendZenSkillsTimerRPC(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;
		
		GetGame().RPCSingleParam(player, ZenSkillConstants.RPC_ZenExpBoostNotify, new Param2<bool, int>(true, ZenGetRemainingSeconds()), true, player.GetIdentity());
	}
}
modded class CarchingResultFishingAction
{
    override void UpdateCatchChance(CatchingContextBase ctx)
    {
        super.UpdateCatchChance(ctx);

        PlayerBase player;
        if (m_Owner) 
			player = PlayerBase.Cast(m_Owner.GetHierarchyRootPlayer());
		
        if (!player) 
			return;

        float add = player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_FISH_CHANCE);

        float before = m_CatchChance;
        float after = before + add;
		
        if (after < 0.0) 
			after = 0.0;
		
        if (after > 1.0) 
			after = 1.0;
		
        SetCatchChance(after);

        #ifdef ZENSKILLSDEBUG
        ZenSkillsPrint("CatchAny:Spawn before=" + before + " add=" + add + " after=" + after);
        #endif
    }
}

modded class CatchingContextFishingRodAction
{
    float GetZenFishPerkPoints01()
    {
        if (!m_Player) 
			return 0;
		
        float perkPct = m_Player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_FISH_CHANCE);

        #ifdef ZENSKILLSDEBUG
        ZenSkillsPrint("FishPerk: perkPoints=" + perkPct);
        #endif
		
        return perkPct;
    }

    // (1) Absolute +x points to bite probability
    override bool ModifySignalProbability(inout float probability)
    {
        bool ok = super.ModifySignalProbability(probability);
        if (!ok) return ok;

        float before = probability;
        float add = GetZenFishPerkPoints01();
        float after = before + add;
		
        if (after < 0.0) 
			after = 0.0;
		
        if (after > 1.0) 
			after = 1.0;
		
        probability = after;

        #ifdef ZENSKILLSDEBUG
        ZenSkillsPrint("Fishing perk: Bite chance before=" + before + " add=" + add + " after=" + after);
        #endif
        return true;
    }
	
	override float GetHookLossChanceModifierClamped()
	{
		float baseChance = super.GetHookLossChanceModifierClamped();
		
		float newChance = baseChance;
		
		float reduceChance = 1 - m_Player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_FISH_HOOKLOSS);
		newChance = Math.Clamp(newChance * reduceChance, 0.0001, 1);
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Fishing GetHookLossChanceModifierClamped - baseChance=" + baseChance + " newChance=" + newChance + " multi=" + reduceChance);
		#endif
		
		return newChance;
	}
	
	override float GetBaitLossChanceModifierClamped()
	{
		float baseChance = super.GetBaitLossChanceModifierClamped();
		
		float newChance = baseChance;
		
		float reduceChance = 1 - m_Player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_FISH_HOOKLOSS);
		newChance = Math.Clamp(newChance * reduceChance, 0, 1);
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Fishing GetBaitLossChanceModifierClamped - baseChance=" + baseChance + " newChance=" + newChance + " multi=" + reduceChance);
		#endif
		
		return newChance;
	}
	
	override protected void TryDamageItems()
	{
		if (!GetGame().IsDedicatedServer())
		{
			super.TryDamageItems();
			return;
		}
		
		float hookHealthBefore = 0;
		if (m_Hook)
		{
			hookHealthBefore = m_Hook.GetHealth();
		}
		
		super.TryDamageItems();
		
		float hookHealthAfter = m_Hook.GetHealth();
		float healthDamageDealt = hookHealthBefore - hookHealthAfter;
		
		if (healthDamageDealt <= 0)
			return;

		float durabilityRefund = m_Player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_FISH_DURABILITY);
		
		if (durabilityRefund <= 0)
			return;
		
		float healthRefund = healthDamageDealt * durabilityRefund;
		m_Hook.AddHealth("","Health", healthRefund);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("TryDamageItems: healthDamageDealt=" + healthDamageDealt + " beforeHP=" + hookHealthBefore + " hookHealthAfter=" + hookHealthAfter + " refundHP=" + healthRefund + " afterHP=" + m_Hook.GetHealth());
		#endif
	}
}
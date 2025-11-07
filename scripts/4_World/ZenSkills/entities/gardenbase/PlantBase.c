modded class PlantBase
{
	protected bool m_ZenNurtureApplied;
	
	bool IsZenNurtureApplied()
	{
		return m_ZenNurtureApplied;
	}
	
	void SetZenNurtureApplied(bool b)
	{
		m_ZenNurtureApplied = b;
	}
	
	void PlantBase()
	{
		RegisterNetSyncVariableBool("m_ZenNurtureApplied");
	}

    void ZenApplyNurturePerks(PlayerBase player)
    {
        if (!player || m_ZenNurtureApplied)
            return;

        // Reduce grow time AND infestation chance
        float reduce01 = Math.Clamp(player.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_PLANT_NURTURE), 0, 1);
        if (reduce01 > 0)
        {
			#ifdef ZENSKILLSDEBUG 
			float m_FullMaturityTimeOrig = m_FullMaturityTime;
			float m_InfestationChanceOrig = m_InfestationChance;
			#endif
			
            float factor = 1 - reduce01;

            float oldStateTime = m_StateChangeTime;
            m_FullMaturityTime = Math.Round(m_FullMaturityTime * factor);

            int denom = m_GrowthStagesCount - 2;
            if (denom < 1) 
				denom = 1;

            m_StateChangeTime = m_FullMaturityTime / denom;

            if (m_PlantState == EPlantState.GROWING && oldStateTime > 0)
            {
                // Preserve progress proportion within current stage
                m_TimeTracker = m_TimeTracker * (m_StateChangeTime / oldStateTime);
            }

            // Lower chance the next stage infests
            m_InfestationChance = Math.Clamp(m_InfestationChance * factor, 0, 1);
			
			#ifdef ZENSKILLSDEBUG 
			string debugMsg = "Grow time & infestation reduction=" + reduce01 + "% ";
			debugMsg = debugMsg + " m_FullMaturityTime=" + m_FullMaturityTime + "/" + m_FullMaturityTimeOrig;
			debugMsg = debugMsg + " m_InfestationChance=" + m_InfestationChance + "/" + m_InfestationChanceOrig;
			ZenSkillsPrint(debugMsg);
			#endif
        }

        // Pre-roll extra crops once (before maturity only)
        if (m_PlantState < EPlantState.MATURE && m_CropsCount > 0)
        {
            float extraChance = Math.Clamp(player.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_EXTRA_PLANTS), 0, 1);
            
			if (extraChance > 0.0)
            {
                int baseCount = m_CropsCount;
                int bonus = 0;
                for (int i = 0; i < baseCount; i++)
                {
                    if (Math.RandomFloat01() < extraChance)
                    {
                        bonus = bonus + 1;
                    }
                }
				
				#ifdef ZENSKILLSDEBUG 
				ZenSkillsPrint("Extra crops added: " + bonus);
				#endif
				
                m_CropsCount = baseCount + bonus;
            }
        }

        m_ZenNurtureApplied = true;
        SetSynchDirty();
    }

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;

		if (!ctx.Read(m_ZenNurtureApplied))
		{
			m_ZenNurtureApplied = false;
		}
		
		return true;
	}

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		
		ctx.Write(m_ZenNurtureApplied);
	}
}

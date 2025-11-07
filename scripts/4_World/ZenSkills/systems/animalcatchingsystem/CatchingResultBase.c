modded class CatchingResultBase
{
    override void UpdateCatchChance(CatchingContextBase ctx)
    {
        super.UpdateCatchChance(ctx);

        TrapSpawnBase trap = TrapSpawnBase.Cast(m_Owner);
        if (!trap) 
		{
			return;
		}
		
		float bonus = trap.GetZenPerkCatchBonus01();
		if (bonus <= 0)
		{
			return;
		}

        float baseChance = m_CatchChance;
        float finalChance = baseChance + bonus;
		
        if (finalChance < 0) 
			finalChance = 0;
		
        if (finalChance > 1) 
			finalChance = 1;

        SetCatchChance(finalChance);

        #ifdef ZENSKILLSDEBUG
        ZenSkillsPrint("[Trap] CatchAnyChance base=" + baseChance + " bonus=" + bonus + " final=" + finalChance);
        #endif
    }
	
	override bool RollChance()
	{
		#ifdef ZENSKILLSDEBUG 
		bool rollChance = super.RollChance(); 
		if (m_Owner)
		{
			ZenSkillsPrint(m_Owner.GetType() + " m_CatchChance01=" + m_CatchChance + " rollChancePassed=" + rollChance);
		}
		return rollChance;
		#endif
		
		return super.RollChance();
	}	
}
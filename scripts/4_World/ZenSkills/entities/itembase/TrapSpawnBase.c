modded class TrapSpawnBase
{
	protected float m_ZenPerkCatchBonus01;
	
	float GetZenPerkCatchBonus01() 
	{ 
		return m_ZenPerkCatchBonus01; 
	}
	
	void TrapSpawnBase()
	{
		m_ZenPerkCatchBonus01 = 0;
	}
	
	// Some traps use ActionDeployHuntingTrap (ie. small fish trap), some use hologram placement - so hook into both.
	override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
	{
		super.OnPlacementComplete(player, position, orientation);
		
		if (GetGame().IsDedicatedServer())
		{
			ZenSkillsDeployTrap(PlayerBase.Cast(player));
		}
	}

	void ZenSkillsDeployTrap(PlayerBase player)
	{
		if (!player)
			return;
		
		m_ZenPerkCatchBonus01 = player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_TRAP_CHANCE);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("ZenSkillsDeployTrap: " + GetType() + " player=" + player.GetIdentity().GetId() + " m_ZenPerkCatchBonus01=" + m_ZenPerkCatchBonus01);
		#endif
	}
	
	override void RunTrappingTimer(float duration, string fnName)
	{
		super.RunTrappingTimer(duration, fnName);
		
		#ifdef ZENSKILLSDEBUG 
		if (m_Timer)
		{
			ZenSkillsPrint(GetType() + " timer check started with repeat delay: " + duration + " seconds");
		}
		#endif
	}
	
	//! PERSISTENCE NOTE:
	// I tried to stay away from onstoreload/save in this mod to make it easy to add/remove without wiping server,
	// but couldn't think of a way around this one. I made an exception because if it's corrupted during OnStoreLoad, 
	// it shouldn't break anything important beyond the trap not setting itself up properly after adding/removing the mod.
	override void OnStoreSave(ParamsWriteContext ctx)
	{
	    super.OnStoreSave(ctx);
	
	    ctx.Write(m_ZenPerkCatchBonus01);
	}
	
	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
	    if (!super.OnStoreLoad(ctx, version))
		{
			return false;
		}
	
	    if (!ctx.Read(m_ZenPerkCatchBonus01)) 
		{
			m_ZenPerkCatchBonus01 = 0.0;
		}
			
	    return true;
	}
}

modded class ActionDeployHuntingTrap
{
	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data);
		
		TrapSpawnBase trap = TrapSpawnBase.Cast(action_data.m_MainItem);
		if (!trap)
			return;
		
		trap.ZenSkillsDeployTrap(action_data.m_Player);
	}
}
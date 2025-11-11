modded class WolfSteakMeat
{
	protected bool m_ZenSkillsNoSalmonella;
	
	void WolfSteakMeat()
	{
		RegisterNetSyncVariableBool("m_ZenSkillsNoSalmonella");
	}
	
	override void SetZenSkillsNoSalmonella(bool b)
	{
		m_ZenSkillsNoSalmonella = b;
		SetSynchDirty();
	}
	
	override void HandleFoodStageChangeAgents(FoodStageType stageOld, FoodStageType stageNew)
	{
		super.HandleFoodStageChangeAgents(stageOld, stageNew);

		if (m_ZenSkillsNoSalmonella && stageNew != FoodStageType.RAW && stageNew != FoodStageType.BURNED)
		{
			RemoveAgent(eAgents.SALMONELLA);
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("REMOVE SALMONELLA FROM " + GetType());
			#endif
		}
	}
	
	override string GetTooltip()
    {
        string description = ConfigGetString("descriptionShort");
		
		if (m_ZenSkillsNoSalmonella)
		{
			description = description + "<br/><br/><color rgba='97,215,124,255'>#STR_ZenSkills_GUI_SafePredatorMeat.</color>";
		}
		
        return description;
    }
	
	//! PERSISTENCE NOTE:
	// I tried to stay away from onstoreload/save in this mod to make it easy to add/remove without wiping server,
	// but couldn't think of a way around this one. I made an exception because hardly anyone would keep wolf/bear steak in storage
	// since it's got a 60% chance of being poisonous and even if it's corrupted during OnStoreLoad, it shouldn't break anything important.
	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		
		ctx.Write(m_ZenSkillsNoSalmonella);
	}
	
	override bool OnStoreLoad(ParamsWriteContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;
		
		if (!ctx.Read(m_ZenSkillsNoSalmonella))
			return false;
			
		return true;
	}
}
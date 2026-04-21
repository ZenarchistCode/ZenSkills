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
	
	//! PERSISTENCE:
	// Use CF_Load/Save because this will NOT break server persistence if the mod is added/removed mid-wipe. 
	// NOTE: storage[] must refer to this mod's CfgMods classname EXACTLY or this won't work.
	
	override void CF_OnStoreSave(CF_ModStorageMap storage)
	{
		super.CF_OnStoreSave(storage);

		auto ctx = storage["ZenSkills"];
		if (!ctx) return;
		
		ctx.Write(m_ZenSkillsNoSalmonella);
	}

	override bool CF_OnStoreLoad(CF_ModStorageMap storage)
	{
		if (!super.CF_OnStoreLoad(storage)) return false;

		auto ctx = storage["ZenSkills"];
		if (!ctx) return true;

		if (ctx.GetVersion() >= 1)
		{
			if (!ctx.Read(m_ZenSkillsNoSalmonella))
			return false;
		}

		return true;
	}
}
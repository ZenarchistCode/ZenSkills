class ZenSkillsEXP_ExpansionQuests
{
	// Config location
	private const static string ModFolder		= "$profile:\\Zenarchist\\";
	private const static string NestedFolder	= "Skills";
	private const static string ConfigName		= "ZenSkillsEXP_ExpansionQuests.json";
	private const static string CURRENT_VERSION	= "1"; // Change this to force structure update.
	string CONFIG_VERSION;
	
	// map: skill, ZenSkillsEXPDef
	ref map<string, ref ZenSkillsEXPDefHolder> ExpDefs;

	// Config data
	void Load()
	{
		SetDefaultValues();
		
		if (GetGame().IsClient())
		{
			return;
		}

		if (FileExist(ModFolder + NestedFolder + "\\" + ConfigName))
		{
			// If config exists, load file
			JsonFileLoader<ZenSkillsEXP_ExpansionQuests>.JsonLoadFile(ModFolder + NestedFolder + "\\" + ConfigName, this);

			// If version mismatch, backup old version of json before replacing it
			if (CONFIG_VERSION != CURRENT_VERSION)
			{
				JsonFileLoader<ZenSkillsEXP_ExpansionQuests>.JsonSaveFile(ModFolder + NestedFolder + "\\" + ConfigName + "_old", this);
			}
			else
			{
				// Config file exists, was loaded successfully, and version matches - stop here.
				return;
			}
		}

		CONFIG_VERSION = CURRENT_VERSION;

		Save();
	}

	void SetDefaultValues()
	{
		// EXP Defs
		ExpDefs = new map<string, ref ZenSkillsEXPDefHolder>();
		
		//------------------------------------------------------------------------------------------------
		//! SURVIVAL
		//------------------------------------------------------------------------------------------------
		ZenSkillsEXPDefHolder survival = new ZenSkillsEXPDefHolder();
		survival.ExpDefs.Insert("ExpansionQuest_ID", new ZenSkillsEXPDef(100, false, false, false));
		ExpDefs.Insert("survival", survival);

		//------------------------------------------------------------------------------------------------
		//! CRAFTING
		//------------------------------------------------------------------------------------------------
		ZenSkillsEXPDefHolder crafting = new ZenSkillsEXPDefHolder();
		crafting.ExpDefs.Insert("ExpansionQuest_ID", new ZenSkillsEXPDef(100, false, false, false));
		ExpDefs.Insert("crafting", crafting);

		//------------------------------------------------------------------------------------------------
		//! HUNTING
		//------------------------------------------------------------------------------------------------
		ZenSkillsEXPDefHolder hunting = new ZenSkillsEXPDefHolder();
		hunting.ExpDefs.Insert("ExpansionQuest_ID", new ZenSkillsEXPDef(100, false, false, false));
		ExpDefs.Insert("hunting", hunting);
		
		//------------------------------------------------------------------------------------------------
		//! GATHERING
		//------------------------------------------------------------------------------------------------
		ZenSkillsEXPDefHolder gathering = new ZenSkillsEXPDefHolder();
		gathering.ExpDefs.Insert("ExpansionQuest_ID", new ZenSkillsEXPDef(100, false, false, false));
		ExpDefs.Insert("gathering", gathering);
	}

	void Save()
	{
		if (GetGame().IsClient())
		{
			return;
		}
		
		if (!FileExist(ModFolder))
		{
			MakeDirectory(ModFolder);
		}

		if (!FileExist(ModFolder + NestedFolder))
		{
			MakeDirectory(ModFolder + NestedFolder);
		}

		JsonFileLoader<ZenSkillsEXP_ExpansionQuests>.JsonSaveFile(ModFolder + NestedFolder + "\\" + ConfigName, this);
	}
}

static ZenSkillsEXP_ExpansionQuests GetZenSkillsEXP_ExpansionQuests()
{
	if (!m_ZenSkillsEXP_ExpansionQuests)
	{
		Print("[ZenSkillsEXP_ExpansionQuests] Init");
		m_ZenSkillsEXP_ExpansionQuests = new ZenSkillsEXP_ExpansionQuests();
		m_ZenSkillsEXP_ExpansionQuests.Load();
	}

	return m_ZenSkillsEXP_ExpansionQuests;
}

ref ZenSkillsEXP_ExpansionQuests m_ZenSkillsEXP_ExpansionQuests;
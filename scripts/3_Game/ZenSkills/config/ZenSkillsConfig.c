// This stores overall broad mod config for skill definitions, server config and sync'ed client config
class ZenSkillsConfig
{
	// Config location
	private const static string ModFolder		= "$profile:\\Zenarchist\\";
	private const static string NestedFolder	= "Skills";
	private const static string ConfigName		= "ZenSkillsConfig.json";
	private const static string CURRENT_VERSION	= "1"; // Change this to force structure update.
	string CONFIG_VERSION;
	
	ref ZenSkillsServerConfig ServerConfig;
	ref ZenSkillsSharedConfig SharedConfig;
	ref map<string, ref ZenSkillDef> SkillDefs;
	
	ref map<string, int> Analytics_AvgTotalExpGainedPerSession;

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
			JsonFileLoader<ZenSkillsConfig>.JsonLoadFile(ModFolder + NestedFolder + "\\" + ConfigName, this);
			
			#ifdef ZENSKILLS 
			Print("[ZenSkills::SERVER] #define ZENSKILLS found");
			#endif

			#ifdef ZENSKILLSDEBUG
			SharedConfig.DebugMode = true;
			#endif

			ZEN_SKILLS_DEBUG_ON = SharedConfig.DebugMode;
			ZenSkillsPrint("[ZenSkills] DEBUG MODE ENABLED: " + ZEN_SKILLS_DEBUG_ON);

			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("#define ZENSKILLSDEBUG found");
			#endif

			GetZenSkillsEXP();

			// If version mismatch, backup old version of json before replacing it
			if (CONFIG_VERSION != CURRENT_VERSION)
			{
				JsonFileLoader<ZenSkillsConfig>.JsonSaveFile(ModFolder + NestedFolder + "\\" + ConfigName + "_old", this);
			}
			else
			{
				// Config file exists, was loaded successfully, and version matches - stop here.
				Save();
				return;
			}
		}

		CONFIG_VERSION = CURRENT_VERSION;

		Save();
	}

	void SetDefaultValues()
	{
		Analytics_AvgTotalExpGainedPerSession = new map<string, int>();
		
		ServerConfig = new ZenSkillsServerConfig();
		SharedConfig = new ZenSkillsSharedConfig();

		SkillDefs = new map<string, ref ZenSkillDef>();

		//! SURVIVAL
		ZenSkillDef survival = new ZenSkillDef("#STR_ZenSkills_Name_Survival", "#STR_ZenSkills_Desc_Survival");
		survival.Perks.Insert("1_1", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name1",  "#STR_ZenSkills_Perk_Survival_Desc1",  	"%", 	5, 10, 15));
		survival.Perks.Insert("1_2", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name2",  "#STR_ZenSkills_Perk_Survival_Desc2",  	"",   	1, 2, 3));
		survival.Perks.Insert("1_3", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name3",  "#STR_ZenSkills_Perk_Survival_Desc3",  	"%",   	10, 20, 30));
		survival.Perks.Insert("2_1", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name4",  "#STR_ZenSkills_Perk_Survival_Desc4",  	"%",   	10, 20, 30));
		survival.Perks.Insert("2_2", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name5",  "#STR_ZenSkills_Perk_Survival_Desc5",  	"%",   	10, 15, 20));
		survival.Perks.Insert("3_1", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name6",  "#STR_ZenSkills_Perk_Survival_Desc6",  	"%",   	20, 40, 60));
		survival.Perks.Insert("3_2", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name7",  "#STR_ZenSkills_Perk_Survival_Desc7",  	"%",   	60, 80, 100));
		survival.Perks.Insert("3_3", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name8",  "#STR_ZenSkills_Perk_Survival_Desc8",  	"%",   	10, 20, 30));
		survival.Perks.Insert("4_1", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name9",  "#STR_ZenSkills_Perk_Survival_Desc9",  	"%",   	5, 10, 15));
		survival.Perks.Insert("4_2", new ZenPerkDef("#STR_ZenSkills_Perk_Survival_Name10", "#STR_ZenSkills_Perk_Survival_Desc10",  	"%", 	5, 10, 15));
		SkillDefs.Insert("survival", survival);
		
		//! CRAFTING
		ZenSkillDef crafting = new ZenSkillDef("#STR_ZenSkills_Name_Crafting", "#STR_ZenSkills_Desc_Crafting");
		crafting.Perks.Insert("1_1", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name1",  "#STR_ZenSkills_Perk_Crafting_Desc1",  	"%",  	5, 10, 15));
		crafting.Perks.Insert("1_2", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name2",  "#STR_ZenSkills_Perk_Crafting_Desc2",  	"%", 	10, 20, 30));
		crafting.Perks.Insert("1_3", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name3",  "#STR_ZenSkills_Perk_Crafting_Desc3",  	"%",  	10, 20, 30));
		crafting.Perks.Insert("2_1", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name4",  "#STR_ZenSkills_Perk_Crafting_Desc4",  	"%",  	5, 10, 15));
		crafting.Perks.Insert("2_2", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name5",  "#STR_ZenSkills_Perk_Crafting_Desc5",  	"%",  	10, 20, 30));
		crafting.Perks.Insert("3_1", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name6",  "#STR_ZenSkills_Perk_Crafting_Desc6",  	"%",  	5, 10, 15));
		crafting.Perks.Insert("3_2", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name7",  "#STR_ZenSkills_Perk_Crafting_Desc7",  	"%",  	10, 20, 30));
		crafting.Perks.Insert("3_3", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name8",  "#STR_ZenSkills_Perk_Crafting_Desc8",  	"%",  	10, 20, 30));
		crafting.Perks.Insert("4_1", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name9",  "#STR_ZenSkills_Perk_Crafting_Desc9",  	"%",  	10, 20, 30));
		crafting.Perks.Insert("4_2", new ZenPerkDef("#STR_ZenSkills_Perk_Crafting_Name10", "#STR_ZenSkills_Perk_Crafting_Desc10",  	"%", 	10, 20,30));
		SkillDefs.Insert("crafting", crafting);
		
		//! HUNTING
		ZenSkillDef hunting = new ZenSkillDef("#STR_ZenSkills_Name_Hunting", "#STR_ZenSkills_Desc_Hunting");
		hunting.Perks.Insert("1_1", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name1",  "#STR_ZenSkills_Perk_Hunting_Desc1",  		"%",  	5, 10, 15));
		hunting.Perks.Insert("1_2", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name2",  "#STR_ZenSkills_Perk_Hunting_Desc2",  		"%",  	10, 20, 30));
		hunting.Perks.Insert("1_3", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name3",  "#STR_ZenSkills_Perk_Hunting_Desc3",  		"%",  	10, 20, 30));
		hunting.Perks.Insert("2_1", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name4",  "#STR_ZenSkills_Perk_Hunting_Desc4",  		"%",  	10, 20, 30));
		hunting.Perks.Insert("2_2", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name5",  "#STR_ZenSkills_Perk_Hunting_Desc5",  		"%",  	10, 20, 30));
		hunting.Perks.Insert("3_1", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name6",  "#STR_ZenSkills_Perk_Hunting_Desc6",  		"%",  	20, 40, 60));
		hunting.Perks.Insert("3_2", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name7",  "#STR_ZenSkills_Perk_Hunting_Desc7",  		"%",  	10, 20, 30));
		hunting.Perks.Insert("3_3", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name8",  "#STR_ZenSkills_Perk_Hunting_Desc8",  		"%",  	10, 20, 30));
		hunting.Perks.Insert("4_1", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name9",  "#STR_ZenSkills_Perk_Hunting_Desc9",  		"%",  	20, 40, 60));
		hunting.Perks.Insert("4_2", new ZenPerkDef("#STR_ZenSkills_Perk_Hunting_Name10", "#STR_ZenSkills_Perk_Hunting_Desc10",  	"m", 	100, 200, 300));
		SkillDefs.Insert("hunting", hunting);
		
		//! GATHERING
		ZenSkillDef gathering = new ZenSkillDef("#STR_ZenSkills_Name_Gathering", "#STR_ZenSkills_Desc_Gathering");
		gathering.Perks.Insert("1_1", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name1",  "#STR_ZenSkills_Perk_Gathering_Desc1",  "%",  	5, 10, 15));
		gathering.Perks.Insert("1_2", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name2",  "#STR_ZenSkills_Perk_Gathering_Desc2",  "%", 	10, 20, 30));
		gathering.Perks.Insert("1_3", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name3",  "#STR_ZenSkills_Perk_Gathering_Desc3",  "%",  	10, 20, 30));
		gathering.Perks.Insert("2_1", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name4",  "#STR_ZenSkills_Perk_Gathering_Desc4",  "%",  	20, 40, 60));
		gathering.Perks.Insert("2_2", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name5",  "#STR_ZenSkills_Perk_Gathering_Desc5",  "%",  	10, 20, 30));
		gathering.Perks.Insert("3_1", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name6",  "#STR_ZenSkills_Perk_Gathering_Desc6",  "%", 	20, 40, 60));
		gathering.Perks.Insert("3_2", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name7",  "#STR_ZenSkills_Perk_Gathering_Desc7",  "%",  	10, 20, 30));
		gathering.Perks.Insert("3_3", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name8",  "#STR_ZenSkills_Perk_Gathering_Desc8",  "%",  	10, 20, 30));
		gathering.Perks.Insert("4_1", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name9",  "#STR_ZenSkills_Perk_Gathering_Desc9",  "%",  	25, 50, 90));
		gathering.Perks.Insert("4_2", new ZenPerkDef("#STR_ZenSkills_Perk_Gathering_Name10", "#STR_ZenSkills_Perk_Gathering_Desc10",  "%", 	20, 40, 60));
		SkillDefs.Insert("gathering", gathering);
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

		JsonFileLoader<ZenSkillsConfig>.JsonSaveFile(ModFolder + NestedFolder + "\\" + ConfigName, this);
	}
}

// Server-side only config
class ZenSkillsServerConfig
{
	int DebugJumpEXP = 500;					// How much random xp to apply to a random skill every time the player jumps IF debug is on
	int AutoSaveTimerSecs = 900;			// Auto-save delay in seconds for each player to auto-save their skill DB (15 mins by default)
	float BaseHandDrillKitSuccess = 0.4;	// Base % chance 0-1 of hand drill kit succeeding to light a fire if NO perk is enabled
	bool EnableAnalytics = true;			// Enable the analytics JSON file to track xp gained per session on average (helps tune skill xp)
	bool EnableDatabaseCache = true;		// Enable the database cache - I recommend leaving this on unless you have a large number of players coming & going
	bool I_AM_USING_MAPLINK = false;		// Only turn this on if you are using my MapLink compatibility .pbo (found in extras folder of the ZenSkills mod)
}

// Server and client config (syncs to client)
class ZenSkillsSharedConfig
{
	bool DebugMode = false;					// Debug mode on/off (DO NOT LEAVE THIS ON FOR LIVE SERVER! ONLY FOR TESTING ON A TEST SERVER)
	bool ShowExpHudOnLeft = false;			// Whether or not to show the EXP hud on the left or right of the screen (for compatibility with mods etc)
	bool AllowHighscores = true;			// Enable the highscores page and functionality
	bool AllowResetPerks = true;			// Allow players to reset their perks from perk screen (does NOT apply to perk reset injector item)
	float EXP_InjectorBoostMulti = 2;		// How much the EXP boost injector applies (2x)
	float EXP_InjectorBoostTime	= 1800;		// How long the EXP boost injector lasts for (30 mins - repeated injections increase the timer but not the multiplier)
	float PercentOfExpLostOnDeath = 0.05;	// How much of total % EXP is lost on death (includes perks, which will be refunded where necessary)
}

static ZenSkillsConfig GetZenSkillsConfig()
{
	if (!m_ZenSkillsConfig)
	{
		Print("[ZenSkillsConfig] Init");
		m_ZenSkillsConfig = new ZenSkillsConfig();
		m_ZenSkillsConfig.Load();
	}

	return m_ZenSkillsConfig;
}

ref ZenSkillsConfig m_ZenSkillsConfig;
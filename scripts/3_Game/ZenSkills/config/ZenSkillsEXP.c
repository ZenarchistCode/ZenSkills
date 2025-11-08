// This manages action-based EXP gains and nerfs config (server-side only)
class ZenSkillsEXP
{
	// Config location
	private const static string ModFolder		= "$profile:\\Zenarchist\\";
	private const static string NestedFolder	= "Skills";
	private const static string ConfigName		= "ZenSkillsEXP.json";
	private const static string CURRENT_VERSION	= "1"; // Change this to force structure update.
	string CONFIG_VERSION;
	
	ref ZenSkillsEXPNerfDef ExpNerfConfig;
	
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
			JsonFileLoader<ZenSkillsEXP>.JsonLoadFile(ModFolder + NestedFolder + "\\" + ConfigName, this);

			// If version mismatch, backup old version of json before replacing it
			if (CONFIG_VERSION != CURRENT_VERSION)
			{
				JsonFileLoader<ZenSkillsEXP>.JsonSaveFile(ModFolder + NestedFolder + "\\" + ConfigName + "_old", this);
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
		// Nerfs
		ExpNerfConfig = new ZenSkillsEXPNerfDef();
		ExpNerfConfig.GlobalExpMultiplier				= 1; 				// applies to all exp awarded
		ExpNerfConfig.EnableExpNerf 					= true; 			// on/off nerf
		ExpNerfConfig.NerfBubbleMaxActions				= 25; 				// 25 actions before nerf kicks in
		ExpNerfConfig.NerfBubbleRadiusMeters 			= 100; 				// actions repeated in 100m bubble are nerfed
		ExpNerfConfig.NerfActionResetTimeMs 			= 10 * 60 * 1000;	// nerf resets in 10 minutes if pla
		ExpNerfConfig.NerfPercentPerActionInBubble		= 0.01;				// nerf to exp gained in % within distance bubble
		ExpNerfConfig.NerfPercentPerActionRepeat		= 0.01;				// nerf to exp gained in % for repeated actions
		ExpNerfConfig.NerfMaxForBubbleAction			= 0.50; 			// max nerf - these stack!
		ExpNerfConfig.NerfMaxForRepeatAction			= 0.50; 			// max nerf - these stack!
		
		// EXP Defs
		ExpDefs = new map<string, ref ZenSkillsEXPDefHolder>();
		
		//------------------------------------------------------------------------------------------------
		//! SURVIVAL
		//------------------------------------------------------------------------------------------------
		ZenSkillsEXPDefHolder survival = new ZenSkillsEXPDefHolder();
		survival.ExpDefs.Insert("ReadSkillBookSurvival", new ZenSkillsEXPDef(1500, false, false, false));
		survival.ExpDefs.Insert("Stabbed_Zombie_Generic", new ZenSkillsEXPDef(100));
		survival.ExpDefs.Insert("Killed_Zombie_Generic", new ZenSkillsEXPDef(10, true));
		survival.ExpDefs.Insert("Killed_Entity_Generic", new ZenSkillsEXPDef(10, true));
		survival.ExpDefs.Insert("Killed_Player", new ZenSkillsEXPDef(100, true));
		survival.ExpDefs.Insert("ActionBandageSelf", new ZenSkillsEXPDef(5));
		survival.ExpDefs.Insert("ActionBandageTarget", new ZenSkillsEXPDef(10));
		survival.ExpDefs.Insert("ActionCookOnStick", new ZenSkillsEXPDef(25));
		survival.ExpDefs.Insert("PurifyWater", new ZenSkillsEXPDef(10));
		survival.ExpDefs.Insert("ChelateWater", new ZenSkillsEXPDef(10));
		survival.ExpDefs.Insert("BloodTest", new ZenSkillsEXPDef(25));
		survival.ExpDefs.Insert("CraftBloodBagIV", new ZenSkillsEXPDef(25));
		survival.ExpDefs.Insert("CraftSalineBagIV", new ZenSkillsEXPDef(25));		
		survival.ExpDefs.Insert("DisinfectItem", new ZenSkillsEXPDef(10));
		survival.ExpDefs.Insert("HandDrillKitSuccess", new ZenSkillsEXPDef(25));
		survival.ExpDefs.Insert("ActionTestBloodSelf", new ZenSkillsEXPDef(25));
		ExpDefs.Insert("survival", survival);

		//------------------------------------------------------------------------------------------------
		//! CRAFTING
		//------------------------------------------------------------------------------------------------
		ZenSkillsEXPDefHolder crafting = new ZenSkillsEXPDefHolder();
		crafting.ExpDefs.Insert("ReadSkillBookCrafting", new ZenSkillsEXPDef(1500, false, false, false));
		crafting.ExpDefs.Insert("GenericCrafting", new ZenSkillsEXPDef(10, true));
		crafting.ExpDefs.Insert("ActionBuildPart", new ZenSkillsEXPDef(10));
		crafting.ExpDefs.Insert("ActionRepairPart", new ZenSkillsEXPDef(10));
		crafting.ExpDefs.Insert("ActionBuildShelter", new ZenSkillsEXPDef(25));
		crafting.ExpDefs.Insert("ActionDismantlePart", new ZenSkillsEXPDef(10));
		ExpDefs.Insert("crafting", crafting);

		//------------------------------------------------------------------------------------------------
		//! HUNTING
		//------------------------------------------------------------------------------------------------
		ZenSkillsEXPDefHolder hunting = new ZenSkillsEXPDefHolder();
		hunting.ExpDefs.Insert("ReadSkillBookHunting", new ZenSkillsEXPDef(1500, false, false, false));
		hunting.ExpDefs.Insert("Killed_Animal_Generic", new ZenSkillsEXPDef(1, true));
		hunting.ExpDefs.Insert("Killed_Animal_UrsusArctos", new ZenSkillsEXPDef(250, true));
		hunting.ExpDefs.Insert("Killed_Animal_CerbusElaphus", new ZenSkillsEXPDef(100, true));
		hunting.ExpDefs.Insert("ActionSkinning", new ZenSkillsEXPDef(50));
		hunting.ExpDefs.Insert("CaughtFish", new ZenSkillsEXPDef(25, true));
		hunting.ExpDefs.Insert("CraftTannedLeather", new ZenSkillsEXPDef(10));
		hunting.ExpDefs.Insert("CleanWeapon", new ZenSkillsEXPDef(10));
		hunting.ExpDefs.Insert("PrepareAnimal", new ZenSkillsEXPDef(5));
		hunting.ExpDefs.Insert("PrepareFish", new ZenSkillsEXPDef(5));
			// NOTE: can decraft & remake these with no/minimal penalty to ingredients
		hunting.ExpDefs.Insert("CraftRabbitSnare", new ZenSkillsEXPDef(5));
		hunting.ExpDefs.Insert("CraftSmallFishTrap", new ZenSkillsEXPDef(5));
		hunting.ExpDefs.Insert("CraftFishNetTrap", new ZenSkillsEXPDef(5));
		ExpDefs.Insert("hunting", hunting);
		
		//------------------------------------------------------------------------------------------------
		//! GATHERING
		//------------------------------------------------------------------------------------------------
		ZenSkillsEXPDefHolder gathering = new ZenSkillsEXPDefHolder();
		gathering.ExpDefs.Insert("ReadSkillBookGathering", new ZenSkillsEXPDef(1500, false, false, false));
		gathering.ExpDefs.Insert("CraftWoodenPlank", new ZenSkillsEXPDef(5));
		gathering.ExpDefs.Insert("ActionSawPlanks", new ZenSkillsEXPDef(5));
		gathering.ExpDefs.Insert("SawWoodenLog", new ZenSkillsEXPDef(5));
		gathering.ExpDefs.Insert("ActionMineBush", new ZenSkillsEXPDef(5));
		gathering.ExpDefs.Insert("ActionMineBushByHand", new ZenSkillsEXPDef(5));
		gathering.ExpDefs.Insert("ActionMineRock", new ZenSkillsEXPDef(25));
		gathering.ExpDefs.Insert("ActionMineTree", new ZenSkillsEXPDef(25));
		gathering.ExpDefs.Insert("ActionHarvestCrops", new ZenSkillsEXPDef(20));
		gathering.ExpDefs.Insert("ActionDigWorms", new ZenSkillsEXPDef(5));
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

		JsonFileLoader<ZenSkillsEXP>.JsonSaveFile(ModFolder + NestedFolder + "\\" + ConfigName, this);
	}
}

//! Purely for organizing the actions in JSON structure under a category based on skill key.
class ZenSkillsEXPDefHolder 
{
	ref map<string, ref ZenSkillsEXPDef> ExpDefs;
	
	void ZenSkillsEXPDefHolder()
	{
		ExpDefs = new map<string, ref ZenSkillsEXPDef>();
	}
}

class ZenSkillsEXPNerfDef
{
	float GlobalExpMultiplier;
	bool EnableExpNerf;
	int NerfBubbleMaxActions;
	float NerfBubbleRadiusMeters;
	float NerfActionResetTimeMs;
	float NerfPercentPerActionInBubble;
	float NerfPercentPerActionRepeat;
	float NerfMaxForBubbleAction;
	float NerfMaxForRepeatAction;
}

class ZenSkillsEXPDef
{
	int EXP;
	bool AllowModifier;
	bool ApplyNerf;
	bool ApplyBoosts;
	
	void ZenSkillsEXPDef(int p_exp, bool p_modifier = false, bool p_nerf = true, bool p_boosts = true)
	{
		EXP = p_exp;
		AllowModifier = p_modifier;
		ApplyNerf = p_nerf;
		ApplyBoosts = p_boosts;
	}
}

static ZenSkillsEXP GetZenSkillsEXP()
{
	if (!m_ZenSkillsEXP)
	{
		Print("[ZenSkillsEXP] Init");
		m_ZenSkillsEXP = new ZenSkillsEXP();
		m_ZenSkillsEXP.Load();
	}

	return m_ZenSkillsEXP;
}

ref ZenSkillsEXP m_ZenSkillsEXP;
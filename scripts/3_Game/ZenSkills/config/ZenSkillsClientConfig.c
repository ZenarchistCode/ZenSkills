// This is purely for client-side settings persistence
class ZenSkillsClientConfig
{
	// Config location
	private const static string ModFolder		= "$profile:\\Zenarchist\\";
	private const static string NestedFolder	= "Skills\\";
	private const static string ConfigName		= "ZenSkillsClientConfig.json";
	private const static string CURRENT_VERSION	= "1"; // Change this to force structure update.
	string CONFIG_VERSION;
	
	bool ShowTutorial;
	bool ShowEXP;
	ref map<string, int> LastPerkNotification;
	
	// Config data
	void Load()
	{
		SetDefaultValues();
		
		if (!GetGame().IsClient())
		{
			return;
		}
		
		string serverPath;
		GetCLIParam("connect", serverPath);
		serverPath.Replace(":", "");
		serverPath.Replace(".", "");
		serverPath = serverPath + "\\";
		
		string configFilePath = ModFolder + serverPath + NestedFolder + ConfigName;

		if (FileExist(configFilePath))
		{
			// If config exists, load file
			JsonFileLoader<ZenSkillsClientConfig>.JsonLoadFile(configFilePath, this);
			Print("[ZenSkills::CLIENT] Loaded client config from path: " + (ModFolder + serverPath + NestedFolder + ConfigName));

			// If version mismatch, backup old version of json before replacing it
			if (CONFIG_VERSION != CURRENT_VERSION)
			{
				JsonFileLoader<ZenSkillsClientConfig>.JsonSaveFile(configFilePath + "_old", this);
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
		
		Print("[ZenSkills::CLIENT] Created client config in path: " + (ModFolder + serverPath + NestedFolder + ConfigName));
	}

	void SetDefaultValues()
	{
		ShowTutorial = true;
		ShowEXP = true;
		LastPerkNotification = new map<string, int>;
	}

	void Save()
	{
		if (!GetGame().IsClient())
		{
			return;
		}
		
		if (!FileExist(ModFolder))
		{
			MakeDirectory(ModFolder);
		}
		
		string serverPath;
		GetCLIParam("connect", serverPath);
		serverPath.Replace(":", "");
		serverPath.Replace(".", "");
		serverPath = serverPath + "\\";
		string configFilePath = ModFolder + serverPath + NestedFolder + ConfigName;
		
		if (!FileExist(ModFolder + serverPath))
		{
			MakeDirectory(ModFolder + serverPath);
		}

		if (!FileExist(ModFolder + serverPath + NestedFolder))
		{
			MakeDirectory(ModFolder + serverPath + NestedFolder);
		}

		JsonFileLoader<ZenSkillsClientConfig>.JsonSaveFile(configFilePath, this);
	}
}

static ZenSkillsClientConfig GetZenSkillsClientConfig()
{
	if (!m_ZenSkillsClientConfig)
	{
		m_ZenSkillsClientConfig = new ZenSkillsClientConfig();
		m_ZenSkillsClientConfig.Load();
	}

	return m_ZenSkillsClientConfig;
}

ref ZenSkillsClientConfig m_ZenSkillsClientConfig;
ref ZenSkillsClientConfig g_ZenSkillsClientConfig;

static ZenSkillsClientConfig GetZenSkillsClientConfig()
{
	if (!g_ZenSkillsClientConfig) GetZenConfigRegister().RegisterConfig(ZenSkillsClientConfig);
	return g_ZenSkillsClientConfig;
}

modded class ZenConfigRegister
{
	override void RegisterPreload()
	{
		super.RegisterPreload(); 
		RegisterType(ZenSkillsClientConfig);
	}
}

class ZenSkillsClientConfig: ZenConfigBase
{
	// -------------------------
	// CONFIG SETTINGS
	// -------------------------
	override void OnRegistered()
	{
		g_ZenSkillsClientConfig = this;
	}
	
	override string 	GetFolderName()       		{ return "Skills"; }
	override string    	GetCurrentVersion()   		{ return "1.29.1"; }
	override bool 		ShouldLoadOnClient()		{ return true; }
	override bool		ShouldLoadOnServer() 		{ return false; }
	override bool		ShouldSyncToClient()		{ return false; }
	
	override bool ReadJson(string path, out string err)
	{
		return JsonFileLoader<ZenSkillsClientConfig>.LoadFile(path, this, err);
	}

	override bool WriteJson(string path, out string err)
	{
		return JsonFileLoader<ZenSkillsClientConfig>.SaveFile(path, this, err);
	}
	
	bool ShowTutorial;
	bool ShowEXP;
	ref map<string, int> LastPerkNotification;
	
	override void SetDefaults()
	{
		ShowTutorial = true;
		ShowEXP = true;
		LastPerkNotification = new map<string, int>;
	}
}
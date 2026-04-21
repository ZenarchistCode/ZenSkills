modded class PluginManager
{
	override void Init()
	{
		super.Init();

		// Plugin classname, client, server
		RegisterPlugin("PluginZenSkills", true, true);
		
		// Server-side only plugins
		if (g_Game.IsDedicatedServer())
		{
			if (GetZenSkillsConfig().ServerConfig.EnableAnalytics)
			{
				RegisterPlugin("PluginZenSkillsAnalytics", false, true);
			}
		}
	}
}
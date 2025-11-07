// Server-side only code
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		Print("[ZenSkills] MissionServer::OnInit");

		// Load server-side config 
		GetZenSkillsConfig();

		#ifdef ZENSKILLSDEBUG 
		if (!GetZenSkillsConfig().SharedConfig.DebugMode)
		{
			GetZenSkillsConfig().SharedConfig.DebugMode = true;
		}
		#endif
		
		// Prep highscores if enabled
		if (GetZenSkillsConfig().SharedConfig.AllowHighscores)
		{
			GetZenSkillsHighscoresDB();
		}
		
		#ifdef ZENMODPACK
		// Just in case someone has my modpack with ZenFireplaceSticks enabled, turn it off as this mod overrides it.
		GetZenModPackConfig().ModEnabled.Set("ZenFireplaceStick", false);
		GetZenModPackConfig().Save();
		Print("[ZenSKills] DISABLE ZENMODPACK FIREPLACE STICK!");
		#endif
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity) 
	{
		super.InvokeOnConnect(player, identity);
		
		if (!player || !identity)
			return;

		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Player connected - loading from DB");
		#endif
		
		player.StartZenSkillsSaveTimer();

		// If maplink compatibility mod is enabled, then that mod syncs to client on player load when received from maplink DB instead.
		if (!GetZenSkillsConfig().ServerConfig.I_AM_USING_MAPLINK)
		{
			ZenSkillsPlayerDB db = GetZenSkillsPlugin().LoadZenSkillsDB(identity.GetId());
			player.SetZenSkillsDB(db);
		}
	}
	
	override PlayerBase OnClientNewEvent(PlayerIdentity identity, vector pos, ParamsReadContext ctx)
	{
		PlayerBase pb = super.OnClientNewEvent(identity, pos, ctx);

		ApplyZenSkillsNewPlayer(pb);
		
		return pb;
	}

	void ApplyZenSkillsNewPlayer(PlayerBase pb)
	{
		if (GetGame().IsDedicatedServer() && pb != null && pb.GetIdentity())
		{
			float expLostPercentage = GetZenSkillsConfig().SharedConfig.PercentOfExpLostOnDeath;
			if (pb.GetZenSkillsDB() && expLostPercentage > 0)
			{
				Print("[ZenSkills] Player respawned: applying EXP loss of " + expLostPercentage.ToString() + "x to " + pb.GetIdentity().GetId());				
				pb.GetZenSkillsDB().ApplyDeathExpPenalty(expLostPercentage);
				pb.SaveZenSkillsDB();
				GetZenSkillsPlugin().ResyncToClientDB(pb.GetIdentity(), pb.GetZenSkillsDB());
			}
		}
	}
	
	override void OnMissionFinish()
	{
		super.OnMissionFinish();
		
		delete m_ZenSkillsConfig;
	}
	
	// If a player just logged out and they were the LAST player to logout, save all DBs.
	// This is a robust way to save databases at server restarts if server runs CFTools and kicks
	// players before a restart, which most high-pop DayZ servers typically do to prevent item duping.
	// It can also trigger if the server is simply quiet and the last player logs out - no harm there either.
	override void PlayerDisconnected(PlayerBase player, PlayerIdentity identity, string uid)
	{
		super.PlayerDisconnected(player, identity, uid);
		
		if (m_LogoutPlayers.Count() > 0)
			return;

		if (GetZenSkillsAnalyticsPlugin())
		{
			GetZenSkillsAnalyticsPlugin().Checkpoint();
			GetZenSkillsAnalyticsPlugin().FinalizeSession();
		}
		
		if (GetZenSkillsConfig().SharedConfig.AllowHighscores && !GetZenSkillsConfig().ServerConfig.I_AM_USING_MAPLINK && GetZenSkillsHighscoresDB())
		{
			GetZenSkillsHighscoresDB().Save();
		}
	}
}
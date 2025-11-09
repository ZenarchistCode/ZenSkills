modded class MissionServer 
{
    override void OnInit() 
    {
        super.OnInit(); 

        if (GetZenSkillsConfig().SharedConfig.AllowHighscores)
		{
			GetZenSkillsHighscoresMapLink(); // triggers .Load() and mirror on receive
		}

		if (!GetZenSkillsConfig().ServerConfig.I_AM_USING_MAPLINK)
		{
			GetZenSkillsConfig().ServerConfig.I_AM_USING_MAPLINK = true;
			GetZenSkillsConfig().Save();
		}
    }

    override void PlayerDisconnected(PlayerBase player, PlayerIdentity identity, string uid)
	{
		super.PlayerDisconnected(player, identity, uid);

		// When server empties, flush remote
		if (m_LogoutPlayers.Count() == 0 && GetZenSkillsConfig().SharedConfig.AllowHighscores)
		{
			ZenSkillsHighscoresMapLink globalDb = GetZenSkillsHighscoresMapLink();

			if (globalDb) 
            {
                globalDb.ImportFromLegacyDBAndSave();
            }
		}
	}
}
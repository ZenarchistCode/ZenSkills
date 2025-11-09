modded class PluginZenSkills 
{
    override void SavePlayerDB(notnull ZenSkillsPlayerDB db, notnull PlayerIdentity id)
	{
		PlayerBase player = PlayerBase.Cast(UniversalApi.FindPlayerByIdentity(id));
        if (player)
        {
            player.ZenSkills_RemoteSave();
        }
	}
}
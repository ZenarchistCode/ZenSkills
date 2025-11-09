modded class PlayerBase
{
	int m_ZenSkills_LastRemoteSave;
	int m_ZenSkills_RemoteRevision;

	void ZenSkills_RemoteLoad()
	{
		if (!GetGame().IsDedicatedServer() || !GetIdentity()) 
			return;

		string playerID = GetIdentity().GetId();

		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("ZenSkills_RemoteLoad triggered for " + playerID);
		#endif 

		// Ensure in-memory DB has safe defaults so gameplay does not NPE before remote arrives
		m_ZenSkillsDB = GetZenSkillsDB();
		if (m_ZenSkillsDB)
		{
			if (!m_ZenSkillsDB.Skills || m_ZenSkillsDB.Skills.Count() == 0)
			{
				m_ZenSkillsDB.SetDefaultValues();
				//m_ZenSkillsDB.SetDefinitions(false);
			}
		}

		ZenSkillsRemoteRecord req = new ZenSkillsRemoteRecord();
		req.BeginLoadFor(this);
	}

	void ZenSkills_RemoteSave()
	{
		if (!GetGame().IsDedicatedServer() || !GetIdentity() || !GetZenSkillsDB()) 
			return;

		if (GetZenSkillsName() == "Survivor" || GetZenSkillsName().Contains("Survivor ("))
			return;

		int timestamp = GetGame().GetTime();
		if (timestamp - m_ZenSkills_LastRemoteSave < 500)
		{
			// Prevent double-saves back-to-back from all these overrides
			return;
		}

		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("ZenSkills_RemoteSave triggered for " + GetIdentity().GetId());
		#endif 

		ZenSkillsRemoteRecord req = new ZenSkillsRemoteRecord();
		req.PublishFromPlayer(this, m_ZenSkills_RemoteRevision + 1);
		m_ZenSkills_RemoteRevision = m_ZenSkills_RemoteRevision + 1;
		m_ZenSkills_LastRemoteSave = timestamp;

		if (GetZenSkillsConfig().SharedConfig.AllowHighscores)
		{
			GetZenSkillsHighscoresDB().UpdateHighscores(GetIdentity().GetId(), GetZenSkillsName(), GetZenSkillsDB().Skills);
		}
	}

	override void SavePlayerToUApi()
	{
		super.SavePlayerToUApi();

		ZenSkills_RemoteSave();
	}

	override void SaveZenSkillsDB() 
	{
		ZenSkills_RemoteSave();
	}

	override void OnConnect()
	{
		super.OnConnect();

		if (GetGame().IsDedicatedServer()) 
		{
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("OnConnect() :: ZenSkills_RemoteLoad");
			#endif

			ZenSkills_RemoteLoad();
		}
	}

	override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);

		ZenSkills_RemoteSave();
    }
}
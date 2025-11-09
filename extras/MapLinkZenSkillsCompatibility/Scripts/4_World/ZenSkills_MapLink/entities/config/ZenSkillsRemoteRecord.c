class ZenSkillsRemoteRecord : UApiConfigBase
{
	int SchemaVersion;
	int Revision;
	int UpdatedAtUnix;
	string LastServerID;

	ref array<string> SkillKeys;
	ref array<int>    SkillEXP;
	ref array<string> PerkSkillKeys;
	ref array<string> PerkKeys;
	ref array<int>    PerkLevels;

	// not serialized
	[NonSerialized()]
	PlayerBase PlayerRef;
	
	[NonSerialized()]
	string PlayerUID;

	override void SetDefaults()
	{
		SchemaVersion = 1;
		Revision = 0;
		UpdatedAtUnix = 0;
		LastServerID = "";
		SkillKeys = new array<string>();
		SkillEXP = new array<int>();
		PerkSkillKeys = new array<string>();
		PerkKeys = new array<string>();
		PerkLevels = new array<int>();
	}

	override string ToJson()
	{
		return UApiJSONHandler<ZenSkillsRemoteRecord>.ToString(this);
	}

	// start an async load for this player
	void BeginLoadFor(PlayerBase player)
	{
		PlayerRef = player;
		PlayerUID = "";
		if (player && player.GetIdentity()) PlayerUID = player.GetIdentity().GetId();

		m_DataReceived = false;
		SetDefaults(); // ensures a valid body if API creates new

		UApi().Rest().PlayerLoad("ZenSkills", PlayerUID, this, this.ToJson());

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkills::Remote] PlayerLoad issued for " + PlayerUID);
		#endif
	}

	// push current live DB to API
	void PublishFromPlayer(PlayerBase player, int targetRevision)
	{
		if (!GetGame().IsDedicatedServer()) return;
		if (!player) return;
		if (!player.GetIdentity()) return;

		PlayerRef = player;
		PlayerUID = player.GetIdentity().GetId();

		ZenSkillsSnapshot snap = new ZenSkillsSnapshot();
		snap.FillFromPlayerDB(player);
		snap.SchemaVersion = 1;
		snap.Revision = targetRevision;
		snap.UpdatedAtUnix = UUtil.GetUnixInt();
		snap.LastServerID = UApiConfig().ServerID;

		FromSnapshot(snap);
		UApi().Rest().PlayerSave("ZenSkills", PlayerUID, this.ToJson());

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkills::Remote] PlayerSave rev=" + targetRevision + " for " + PlayerUID);
		#endif
	}

	// UApi callback
	override void OnSuccess(string data, int dataSize)
	{
		if (UApiJSONHandler<ZenSkillsRemoteRecord>.FromString(data, this))
		{
			OnDataReceive();
		}
		else
		{
			MLLog.Err("[ZenSkills::Remote] Invalid data");
		}
	}

	override void OnDataReceive()
	{
		SetDataReceived(true);

		if (!PlayerRef)
			return;

		bool hasRows = false;
		if (SkillKeys && SkillKeys.Count() > 0) hasRows = true;

		if (hasRows)
		{
			ZenSkillsSnapshot snap = new ZenSkillsSnapshot();
			ToSnapshot(snap);
			snap.ApplyToPlayerDB(PlayerRef);

			PlayerRef.m_ZenSkills_RemoteRevision = Revision;

			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[ZenSkills::Remote] Applied remote db rev=" + Revision + " UID=" + PlayerUID);
			#endif
		}
		else
		{
			// First-time player: create a remote record from current defaults
			ZenSkillsSnapshot fresh = new ZenSkillsSnapshot();
			fresh.FillFromPlayerDB(PlayerRef);
			fresh.Revision = 1;
			fresh.UpdatedAtUnix = UUtil.GetUnixInt();
			fresh.LastServerID = UApiConfig().ServerID;

			FromSnapshot(fresh);
			UApi().Rest().PlayerSave("ZenSkills", PlayerUID, this.ToJson());

			PlayerRef.m_ZenSkills_RemoteRevision = 1;
			fresh.ApplyToPlayerDB(PlayerRef);

			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[ZenSkills::Remote] Created new remote db record UID=" + PlayerUID);
			#endif
		}
	}

	// helpers to convert
	void FromSnapshot(ZenSkillsSnapshot s)
	{
		SchemaVersion = s.SchemaVersion;
		Revision = s.Revision;
		UpdatedAtUnix = s.UpdatedAtUnix;
		LastServerID = s.LastServerID;

		SkillKeys = new array<string>();
		SkillEXP = new array<int>();
		PerkSkillKeys = new array<string>();
		PerkKeys = new array<string>();
		PerkLevels = new array<int>();

		for (int i = 0; i < s.SkillKeys.Count(); i++)
		{
			SkillKeys.Insert(s.SkillKeys.Get(i));
			SkillEXP.Insert(s.SkillEXP.Get(i));
		}
		for (int j = 0; j < s.PerkSkillKeys.Count(); j++)
		{
			PerkSkillKeys.Insert(s.PerkSkillKeys.Get(j));
			PerkKeys.Insert(s.PerkKeys.Get(j));
			PerkLevels.Insert(s.PerkLevels.Get(j));
		}
	}

	void ToSnapshot(out ZenSkillsSnapshot s)
	{
		s = new ZenSkillsSnapshot();
		s.SchemaVersion = SchemaVersion;
		s.Revision = Revision;
		s.UpdatedAtUnix = UpdatedAtUnix;
		s.LastServerID = LastServerID;

		for (int i = 0; i < SkillKeys.Count(); i++)
		{
			s.SkillKeys.Insert(SkillKeys.Get(i));
			s.SkillEXP.Insert(SkillEXP.Get(i));
		}
		for (int j = 0; j < PerkSkillKeys.Count(); j++)
		{
			s.PerkSkillKeys.Insert(PerkSkillKeys.Get(j));
			s.PerkKeys.Insert(PerkKeys.Get(j));
			s.PerkLevels.Insert(PerkLevels.Get(j));
		}
	}
}

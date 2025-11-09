class ZenSkillsSnapshot
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

	void ZenSkillsSnapshot()
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

	void Clear()
	{
		SkillKeys.Clear();
		SkillEXP.Clear();
		PerkSkillKeys.Clear();
		PerkKeys.Clear();
		PerkLevels.Clear();
	}

	// DENSE export: includes zero EXP and zero-level perks
	void FillFromPlayerDB(PlayerBase player)
	{
		Clear();
		if (!player) return;

		ZenSkillsPlayerDB dbRef = player.GetZenSkillsDB();
		if (!dbRef) return;

		auto cfg = GetZenSkillsConfig();
		if (!cfg || !cfg.SkillDefs) return;

		foreach (string skillKey, ZenSkillDef skillDef : cfg.SkillDefs)
		{
			ZenSkill liveSkill = null;
			if (dbRef.Skills) liveSkill = dbRef.Skills.Get(skillKey);

			int expValue = 0;
			if (liveSkill) expValue = liveSkill.EXP;

			SkillKeys.Insert(skillKey);
			SkillEXP.Insert(expValue);

			if (skillDef && skillDef.Perks)
			{
				foreach (string perkKey, ZenPerkDef perkDef : skillDef.Perks)
				{
					int levelValue = 0;
					if (liveSkill && liveSkill.Perks)
					{
						ZenPerk livePerk = liveSkill.Perks.Get(perkKey);
						if (livePerk) levelValue = livePerk.Level;
					}
					PerkSkillKeys.Insert(skillKey);
					PerkKeys.Insert(perkKey);
					PerkLevels.Insert(levelValue);
				}
			}
		}
	}

	// Overwrite live DB
	void ApplyToPlayerDB(PlayerBase player)
	{
		if (!player || !player.GetIdentity()) 
		{
			Error("ApplyToPlayerDB: NO PLAYER!");
			return;
		}

		ZenSkillsPlayerDB dbRef = player.GetZenSkillsDB();
		if (!dbRef)
		{
			dbRef = new ZenSkillsPlayerDB(player.GetIdentity().GetId());
		}

		if (!dbRef.Skills || dbRef.Skills.Count() == 0)
		{
			dbRef.SetDefaultValues();
		}
		else
		{
			foreach (string resetSkillKey, ZenSkill resetSkill : dbRef.Skills)
			{
				if (!resetSkill) continue;
				resetSkill.EXP = 0;
				if (resetSkill.Perks)
				{
					foreach (string resetPerkKey, ZenPerk resetPerk : resetSkill.Perks)
					{
						if (resetPerk) resetPerk.Level = 0;
					}
				}
			}
		}

		for (int i = 0; i < SkillKeys.Count(); i++)
		{
			string sKey = SkillKeys.Get(i);
			int expValue = SkillEXP.Get(i);

			ZenSkill sObj = dbRef.Skills.Get(sKey);
			if (!sObj)
			{
				sObj = new ZenSkill();
				if (!dbRef.Skills) dbRef.Skills = new map<string, ref ZenSkill>();
				dbRef.Skills.Set(sKey, sObj);
			}
			sObj.EXP = expValue;
		}

		int nA = PerkSkillKeys.Count();
		int nB = PerkKeys.Count();
		int nC = PerkLevels.Count();
		int n = nA;
		if (nB < n) n = nB;
		if (nC < n) n = nC;

		for (int j = 0; j < n; j++)
		{
			string sKey2 = PerkSkillKeys.Get(j);
			string pKey = PerkKeys.Get(j);
			int levelValue = PerkLevels.Get(j);

			ZenSkill s2 = dbRef.Skills.Get(sKey2);
			if (!s2)
			{
				s2 = new ZenSkill();
				if (!dbRef.Skills) 
					dbRef.Skills = new map<string, ref ZenSkill>();

				dbRef.Skills.Set(sKey2, s2);
			}

			if (!s2.Perks) 
				s2.Perks = new map<string, ref ZenPerk>();

			ZenPerk pObj = s2.Perks.Get(pKey);
			if (!pObj)
			{
				pObj = new ZenPerk();
				s2.Perks.Set(pKey, pObj);
			}

			pObj.Level = levelValue;
		}

		dbRef.SetDefinitions();
		player.SetZenSkillsDB(dbRef);
	}
}
// =====================================================================
// ZenSkills highscores (GLOBAL via MapLink) — no local files
// Key in API: "ZenSkills_Highscores"
// =====================================================================
class ZenSkillsHighscoresMapLink extends UApiConfigBase
{
	int MaxEntries;
	ref array<string> HighscoresIgnoreList; // A list of player IDs to ignore from the highscores (useful to keep your admin/test accounts off the list)

	// Flat columns (row-aligned)
	ref array<string> HS_SkillKeys;
	ref array<string> HS_PlayerUIDs;
	ref array<string> HS_PlayerNames;
	ref array<int>    HS_Levels;

	// ---------- UApiConfigBase ----------
	override void SetDefaults()
	{
		MaxEntries = 50;
		InitArrays(false);
	}

	override void Load()
	{
		if (!m_DataReceived)
		{
			SetDefaults();
		}

		m_DataReceived = false;
		UApi().Rest().GlobalsLoad("ZenSkills_Highscores", this, this.ToJson());
	}

	override void Save()
	{
		if (GetGame().IsDedicatedServer())
		{
			UApi().Rest().GlobalsSave("ZenSkills_Highscores", this.ToJson());

			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("[ZenSkillsHighscoresMapLink] Save() - entries=" + HS_PlayerUIDs.Count());
			#endif
		}
	}

	override string ToJson()
	{
		return UApiJSONHandler<ZenSkillsHighscoresMapLink>.ToString(this);
	}

	override void OnSuccess(string data, int dataSize)
	{
		if (UApiJSONHandler<ZenSkillsHighscoresMapLink>.FromString(data, this))
		{
			OnDataReceive();
		}
		else
		{
			MLLog.Err("[Highscores-ML] Invalid data in OnSuccess");
		}
	}

	override void OnDataReceive()
	{
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("[ZenSkillsHighscoresMapLink] OnDataReceive()");
		#endif

		// Keep remote tidy and push into legacy in-memory DB
		SortAndCullAll();
		MirrorIntoLegacyDB();
		SetDataReceived();

		GetZenSkillsHighscoresDB().HighscoresIgnoreList.Clear();
		foreach (string s : HighscoresIgnoreList)
		{
			GetZenSkillsHighscoresDB().HighscoresIgnoreList.Insert(s);
		}
	}

	// Convenience for UI/debug
	int RowCount()
	{
		if (!HS_SkillKeys) return 0;
		return HS_SkillKeys.Count();
	}

	// ---------- Internal: arrays/maps ----------
	protected void InitArrays(bool clearExisting)
	{
		if (!HS_SkillKeys)   HS_SkillKeys   = new array<string>();
		if (!HS_PlayerUIDs)  HS_PlayerUIDs  = new array<string>();
		if (!HS_PlayerNames) HS_PlayerNames = new array<string>();
		if (!HS_Levels)      HS_Levels      = new array<int>();

		if (clearExisting)
		{
			HS_SkillKeys.Clear();
			HS_PlayerUIDs.Clear();
			HS_PlayerNames.Clear();
			HS_Levels.Clear();
		}

		if (!HighscoresIgnoreList)
		{
			HighscoresIgnoreList = new array<string>();
		}
		
		if (HighscoresIgnoreList.Count() == 0)
		{
			HighscoresIgnoreList.Insert("4s_12UDE-PKYemc7adlZyKGVSrwzIMW0T32Q39CerkE=");
		}
	}

	protected map<string, ref array<ref ZenSkillsHighscoreDef>> BuildMapView()
	{
		map<string, ref array<ref ZenSkillsHighscoreDef>> outMap = new map<string, ref array<ref ZenSkillsHighscoreDef>>();
		if (!HS_SkillKeys || !HS_PlayerUIDs || !HS_PlayerNames || !HS_Levels)
			return outMap;

		int limit = HS_SkillKeys.Count();
		int a = HS_PlayerUIDs.Count();
		int b = HS_PlayerNames.Count();
		int c = HS_Levels.Count();

		if (a < limit) limit = a;
		if (b < limit) limit = b;
		if (c < limit) limit = c;

		for (int i = 0; i < limit; i++)
		{
			string skillKey = HS_SkillKeys.Get(i);

			array<ref ZenSkillsHighscoreDef> bucket = outMap.Get(skillKey);
			if (!bucket)
			{
				bucket = new array<ref ZenSkillsHighscoreDef>();
				outMap.Set(skillKey, bucket);
			}

			ZenSkillsHighscoreDef row = new ZenSkillsHighscoreDef();
			row.PlayerUID  = HS_PlayerUIDs.Get(i);
			row.PlayerName = HS_PlayerNames.Get(i);
			row.Level      = HS_Levels.Get(i);
			bucket.Insert(row);
		}

		return outMap;
	}

	protected void FlattenFromMap(map<string, ref array<ref ZenSkillsHighscoreDef>> table)
	{
		InitArrays(true);

		for (int i = 0; i < table.Count(); i++)
		{
			string skillKey = table.GetKey(i);
			array<ref ZenSkillsHighscoreDef> bucket = table.GetElement(i);
			if (!bucket) continue;

			for (int j = 0; j < bucket.Count(); j++)
			{
				ZenSkillsHighscoreDef row = bucket.Get(j);
				if (!row) continue;

				HS_SkillKeys.Insert(skillKey);
				HS_PlayerUIDs.Insert(row.PlayerUID);
				HS_PlayerNames.Insert(row.PlayerName);
				HS_Levels.Insert(row.Level);
			}
		}
	}

	// ---------- Internal: sort/cull/mirror ----------
	protected ZenSkillsHighscoreDef FindByUID(array<ref ZenSkillsHighscoreDef> bucket, string uid)
	{
		if (!bucket) return null;
		for (int i = 0; i < bucket.Count(); i++)
		{
			ZenSkillsHighscoreDef row = bucket.Get(i);
			if (row && row.PlayerUID == uid)
				return row;
		}
		return null;
	}

	protected void SortAndCullAll()
	{
		map<string, ref array<ref ZenSkillsHighscoreDef>> table = BuildMapView();
		SortAndCull(table);
		FlattenFromMap(table);
	}

	protected void SortAndCull(map<string, ref array<ref ZenSkillsHighscoreDef>> table)
	{
		if (!table) return;

		for (int i = 0; i < table.Count(); i++)
		{
			array<ref ZenSkillsHighscoreDef> bucket = table.GetElement(i);
			if (!bucket) continue;

			SelectionSort_LevelDesc_UIDAsc(bucket);
			CullToMax(bucket, MaxEntries);
		}
	}

	protected void SelectionSort_LevelDesc_UIDAsc(array<ref ZenSkillsHighscoreDef> bucket)
	{
		if (!bucket) return;
		int n = bucket.Count();

		for (int i = 0; i < n - 1; i++)
		{
			int bestIndex = i;
			ZenSkillsHighscoreDef bestRow = bucket.Get(i);

			for (int j = i + 1; j < n; j++)
			{
				ZenSkillsHighscoreDef candidate = bucket.Get(j);
				if (!candidate) continue;

				bool candidateIsBetter = false;

				if (!bestRow)
				{
					candidateIsBetter = true;
				}
				else
				{
					if (candidate.Level > bestRow.Level)
					{
						candidateIsBetter = true;
					}
					else
					{
						if (candidate.Level == bestRow.Level)
						{
							if (candidate.PlayerUID < bestRow.PlayerUID)
							{
								candidateIsBetter = true;
							}
						}
					}
				}

				if (candidateIsBetter)
				{
					bestIndex = j;
					bestRow = candidate;
				}
			}

			if (bestIndex != i)
			{
				ZenSkillsHighscoreDef tmp = bucket.Get(i);
				bucket.Set(i, bucket.Get(bestIndex));
				bucket.Set(bestIndex, tmp);
			}
		}
	}

	protected void CullToMax(array<ref ZenSkillsHighscoreDef> bucket, int maxEntries)
	{
		if (!bucket) return;
		if (maxEntries <= 0) return;

		while (bucket.Count() > maxEntries)
		{
			bucket.Remove(bucket.Count() - 1);
		}
	}

	// Push global data into the legacy in-memory DB so existing UI/RPC keeps working
	void MirrorIntoLegacyDB()
	{
		ZenSkillsHighscoresDB legacy = GetZenSkillsHighscoresDB();
		if (!legacy) 
			return;

		legacy.SetDefaultValues();
		legacy.MaxEntries = MaxEntries;

		if (!HS_SkillKeys || !HS_PlayerUIDs || !HS_PlayerNames || !HS_Levels)
			return;

		int limit = HS_SkillKeys.Count();
		int a = HS_PlayerUIDs.Count();
		int b = HS_PlayerNames.Count();
		int c = HS_Levels.Count();

		if (a < limit) limit = a;
		if (b < limit) limit = b;
		if (c < limit) limit = c;

		for (int i = 0; i < limit; i++)
		{
			string skillKey = HS_SkillKeys.Get(i);

			array<ref ZenSkillsHighscoreDef> bucket = legacy.Highscores.Get(skillKey);
			if (!bucket)
			{
				bucket = new array<ref ZenSkillsHighscoreDef>();
				legacy.Highscores.Set(skillKey, bucket);
			}

			ZenSkillsHighscoreDef row = new ZenSkillsHighscoreDef();
			row.PlayerUID  = HS_PlayerUIDs.Get(i);
			row.PlayerName = HS_PlayerNames.Get(i);
			row.Level      = HS_Levels.Get(i);
			bucket.Insert(row);
		}

		legacy.SortArrays();

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[Highscores-ML] Mirrored into legacy: skills=" + legacy.Highscores.Count());
		#endif
	}

	// Copy legacy in-memory map -> flatten into this global object, then Save().
	// Returns true if any rows were imported.
	bool ImportFromLegacyDBAndSave(bool clearExisting = true)
	{
		ZenSkillsHighscoresDB legacy = GetZenSkillsHighscoresDB();
		if (!legacy)
		{
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[Highscores-ML] Import: legacy DB is null");
			#endif
			return false;
		}
		
		if (!legacy.Highscores)
		{
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[Highscores-ML] Import: legacy.Highscores is null");
			#endif
			return false;
		}

		if (clearExisting)
		{
			InitArrays(true);
		}

		// Mirror basic setting
		MaxEntries = legacy.MaxEntries;

		// Flatten legacy map -> ML columns
		for (int skillIndex = 0; skillIndex < legacy.Highscores.Count(); skillIndex++)
		{
			string skillKey = legacy.Highscores.GetKey(skillIndex);
			array<ref ZenSkillsHighscoreDef> rows = legacy.Highscores.GetElement(skillIndex);
			if (!rows)
				continue;

			for (int rowIndex = 0; rowIndex < rows.Count(); rowIndex++)
			{
				ZenSkillsHighscoreDef row = rows.Get(rowIndex);
				if (!row)
					continue;

				HS_SkillKeys.Insert(skillKey);
				HS_PlayerUIDs.Insert(row.PlayerUID);
				HS_PlayerNames.Insert(row.PlayerName);
				HS_Levels.Insert(row.Level);
			}
		}

		// Normalize then persist
		SortAndCullAll();
		Save();

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[Highscores-ML] ImportFromLegacyDBAndSave: rows=" + RowCount());
		#endif

		return RowCount() > 0;
	}
}

// Singleton accessor
static ZenSkillsHighscoresMapLink GetZenSkillsHighscoresMapLink()
{
	if (!m_ZenSkillsHighscoresMapLink)
	{
		MLLog.Debug("[Highscores-ML] Init global");
		m_ZenSkillsHighscoresMapLink = new ZenSkillsHighscoresMapLink();
		m_ZenSkillsHighscoresMapLink.Load();
	}
	return m_ZenSkillsHighscoresMapLink;
}

ref ZenSkillsHighscoresMapLink m_ZenSkillsHighscoresMapLink;
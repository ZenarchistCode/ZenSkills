// This stores the highscores data (server-side read/write, client-side sync read only)
class ZenSkillsHighscoresDB
{
	// Config location
	private const static string ModFolder		= "$profile:\\Zenarchist\\";
	private const static string NestedFolder	= "Skills";
	private const static string ConfigName		= "ZenSkillsHighscoresDB.json";
	private const static string CURRENT_VERSION	= "1"; // Change this to force structure update.
	string CONFIG_VERSION;
	
	int MaxEntries;
	ref array<string> HighscoresIgnoreList; 	// A list of player IDs to ignore from the highscores (useful to keep your admin/test accounts off the list)

	// skillKey, highscores holder
	ref map<string, ref array<ref ZenSkillsHighscoreDef>> Highscores;
	
	// Config data
	void Load()
	{
		SetDefaultValues();

		if (GetGame().IsClient())
		{
			return;
		}

		if (GetZenSkillsConfig().ServerConfig.I_AM_USING_MAPLINK)
		{
			return; // handled by MapLink in this case
		}

		if (FileExist(ModFolder + NestedFolder + "\\" + ConfigName))
		{
			// If config exists, load file
			JsonFileLoader<ZenSkillsHighscoresDB>.JsonLoadFile(ModFolder + NestedFolder + "\\" + ConfigName, this);

			// If version mismatch, backup old version of json before replacing it
			if (CONFIG_VERSION != CURRENT_VERSION)
			{
				JsonFileLoader<ZenSkillsHighscoresDB>.JsonSaveFile(ModFolder + NestedFolder + "\\" + ConfigName + "_old", this);
			}
			else
			{
				// Config file exists, was loaded successfully, and version matches - stop here.
				SortArrays();
				return;
			}
		}

		CONFIG_VERSION = CURRENT_VERSION;

		Save();
	}

	void SetDefaultValues()
	{
		MaxEntries = 50;
		Highscores = new map<string, ref array<ref ZenSkillsHighscoreDef>>();
		HighscoresIgnoreList = new array<string>();
		HighscoresIgnoreList.Insert("");
	}

	void Save()
	{
		if (GetGame().IsClient())
		{
			return;
		}

		if (!FileExist(ModFolder))
		{
			MakeDirectory(ModFolder);
		}

		if (!FileExist(ModFolder + NestedFolder))
		{
			MakeDirectory(ModFolder + NestedFolder);
		}

		JsonFileLoader<ZenSkillsHighscoresDB>.JsonSaveFile(ModFolder + NestedFolder + "\\" + ConfigName, this);
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Saved Highscores");
		#endif
	}
	
	void UpdateHighscores(string playerID, string playerName, map<string, ref ZenSkill> skills)
	{
		if ((HighscoresIgnoreList && HighscoresIgnoreList.Find(playerID) != -1) || playerID.Contains(".json"))
			return;

		if (playerName == "Survivor" || playerName.Contains("Survivor ("))
			return;
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("UpdateHighscores: " + playerID + " playerName=" + playerName);
		#endif
		
		foreach (string skillKey, ZenSkill skill : skills)
		{
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("  SKILLKEY=" + skillKey);
			#endif

			if (!Highscores.Get(skillKey))
			{
				Highscores.Set(skillKey, new array<ref ZenSkillsHighscoreDef>());
				
				#ifdef ZENSKILLSDEBUG 
				ZenSkillsPrint("  Create new array for skillKey");
				#endif
			}
			#ifdef ZENSKILLSDEBUG 
			else 
			{
				ZenSkillsPrint("  Found existing array");
			}
			#endif 
			
			ZenSkillsHighscoreDef playerScore;
			
			for (int i = 0; i < Highscores.Get(skillKey).Count(); i++)
			{
				if (Highscores.Get(skillKey).Get(i).PlayerUID == playerID)
				{
					playerScore = Highscores.Get(skillKey).Get(i);
					break;
				}
			}
			
			if (!playerScore)
			{
				#ifdef ZENSKILLSDEBUG 
				ZenSkillsPrint("  Found existing entry for " + skillKey);
				#endif
				
				playerScore = new ZenSkillsHighscoreDef(playerID);
				Highscores.Get(skillKey).Insert(playerScore);
			}
			
			playerScore.PlayerUID = playerID;
			playerScore.PlayerName = playerName;
			playerScore.Level = skill.CountPerksUnused() + skill.CountPerksUnlocked();
			
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("  Saved level: " + playerScore.Level + " - totalEntries=" + Highscores.Get(skillKey).Count());
			#endif
		}
		
		SortArrays();
	}
	
	// In-place selection sort by Level DESC (ties broken by PlayerUID ASC for stability)
	void SortHighscoreArrayByLevelDesc(array<ref ZenSkillsHighscoreDef> arr)
	{
		if (!arr) return;
		int n = arr.Count();
	
		for (int i = 0; i < n - 1; i++)
		{
			int best = i;
			ZenSkillsHighscoreDef bestItem = arr.Get(i);
	
			for (int j = i + 1; j < n; j++)
			{
				ZenSkillsHighscoreDef cand = arr.Get(j);
				if (!cand) continue;
				if (!bestItem)
				{
					best = j;
					bestItem = cand;
					continue;
				}
	
				bool better = false;
				if (cand.Level > bestItem.Level)
					better = true;
				else if (cand.Level == bestItem.Level && cand.PlayerUID < bestItem.PlayerUID)
					better = true;
	
				if (better)
				{
					best = j;
					bestItem = cand;
				}
			}
	
			if (best != i)
			{
				ZenSkillsHighscoreDef tmp = arr.Get(i);
				arr.Set(i, arr.Get(best));
				arr.Set(best, tmp);
			}
		}
	}
	
	void CullArrayToMax(array<ref ZenSkillsHighscoreDef> arr)
	{
		if (!arr) return;
		if (MaxEntries <= 0) return;
	
		while (arr.Count() > MaxEntries)
		{
			arr.Remove(arr.Count() - 1); // remove lowest (end of list after sort)
		}
	}
	
	// Call this after loading OR after UpdateHighscores() if you want it always tidy
	void SortArrays()
	{
		if (!Highscores) return;
	
		for (int i = 0; i < Highscores.Count(); i++)
		{
			string skillKey = Highscores.GetKey(i);
			array<ref ZenSkillsHighscoreDef> arr = Highscores.GetElement(i);
			if (!arr) continue;
	
			// Remove nulls defensively
			for (int n = arr.Count() - 1; n >= 0; n--)
			{
				if (!arr.Get(n))
					arr.Remove(n);
			}
	
			SortHighscoreArrayByLevelDesc(arr);
	
			int before = arr.Count();
			CullArrayToMax(arr);
	
			#ifdef ZENSKILLSDEBUG
			if (before != arr.Count())
				ZenSkillsPrint("[Highscores] Culled " + skillKey + " from " + before + " to " + arr.Count());
			if (arr.Count() > 0)
				ZenSkillsPrint("[Highscores] Top " + skillKey + ": " + arr.Get(0).PlayerName + " (lvl " + arr.Get(0).Level + ")");
			#endif
		}
	}
}

class ZenSkillsHighscoreDef
{
	string PlayerUID;
	string PlayerName;
	int Level;
	
	void ZenSkillsHighscoreDef(string pid = "")
	{
		PlayerUID = pid;
	}
}

static ZenSkillsHighscoresDB GetZenSkillsHighscoresDB()
{
	if (!m_ZenSkillsHighscoresDB)
	{
		Print("[ZenSkillsHighscoresDB] Init");
		m_ZenSkillsHighscoresDB = new ZenSkillsHighscoresDB();
		m_ZenSkillsHighscoresDB.Load();
	}

	return m_ZenSkillsHighscoresDB;
}

ref ZenSkillsHighscoresDB m_ZenSkillsHighscoresDB;
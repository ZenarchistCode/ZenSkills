ref ZenSkillsHighscoresDB g_ZenSkillsHighscoresDB;

static ZenSkillsHighscoresDB GetZenSkillsHighscoresDB()
{
	if (!g_ZenSkillsHighscoresDB) GetZenConfigRegister().RegisterConfig(ZenSkillsHighscoresDB);
	return g_ZenSkillsHighscoresDB;
}

modded class ZenConfigRegister
{
	override void RegisterPreload()
	{
		super.RegisterPreload(); 
		RegisterType(ZenSkillsHighscoresDB);
	}
}

// This stores the highscores data (server-side read/write, client-side sync read only)
class ZenSkillsHighscoresDB: ZenConfigBase
{
	// -------------------------
	// CONFIG SETTINGS
	// -------------------------
	override void OnRegistered()
	{
		g_ZenSkillsHighscoresDB = this;
	}
	
	override string 	GetFolderName()       		{ return "Skills\\DB\\"; }
	override string    	GetCurrentVersion()   		{ return "1.29.1"; }
	override bool		ShouldLoadOnServer() 		{ return !GetZenSkillsConfig().ServerConfig.I_AM_USING_MAPLINK; }
	
	override bool ReadJson(string path, out string err)
	{
		return JsonFileLoader<ZenSkillsHighscoresDB>.LoadFile(path, this, err);
	}

	override bool WriteJson(string path, out string err)
	{
		return JsonFileLoader<ZenSkillsHighscoresDB>.SaveFile(path, this, err);
	}

	// -------------------------
	// CONFIG VARIABLES
	// -------------------------
	int MaxEntries;
	ref array<string> HighscoresIgnoreList = new array<string>; 	// A list of player IDs to ignore from the highscores (useful to keep your admin/test accounts off the list)

	// skillKey, highscores holder
	ref map<string, ref array<ref ZenSkillsHighscoreDef>> Highscores = new map<string, ref array<ref ZenSkillsHighscoreDef>>;

	override void SetDefaults()
	{
		MaxEntries = 50;
		Highscores = new map<string, ref array<ref ZenSkillsHighscoreDef>>;
		HighscoresIgnoreList = new array<string>();
		HighscoresIgnoreList.Insert("");
	}
	
	override void AfterLoad()
	{
		super.AfterLoad();
		
		SortArrays();
	}

	void UpdateHighscores(string playerID, string playerName, map<string, ref ZenSkill> skills)
	{
		if ((HighscoresIgnoreList && HighscoresIgnoreList.Find(playerID) != -1) || playerID.Contains(".json"))
			return;
		
		if (GetZenCoreConfig().IsAdmin(playerID))
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
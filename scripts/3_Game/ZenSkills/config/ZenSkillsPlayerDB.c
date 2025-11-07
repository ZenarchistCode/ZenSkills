// This stores all of the player's skill data (server-side read/write, client-side sync read-only)
class ZenSkillsPlayerDB
{
	static bool RECEIVED_DATA = false; // client-side only.

	// Config location
	const static string ModFolder			= "$profile:\\Zenarchist\\";
	const static string NestedFolder		= "Skills\\";
	const static string DBFolder			= "PlayerDB";
	const static string CURRENT_VERSION		= "1"; // Change this to force structure update. WARNING: THIS WILL WIPE ALL PLAYER PERKS & EXP!
	string CONFIG_VERSION;

	string PlayerName;
	string PlayerUID;
	ref map<string, ref ZenSkill> Skills;
	
	void ZenSkillsPlayerDB()
	{
		Skills = new map<string, ref ZenSkill>();
	}

	// Config data
	void Load(string configName)
	{
		if (GetGame().IsClient())
		{
			return;
		}

		if (GetZenSkillsConfig().ServerConfig.I_AM_USING_MAPLINK)
		{
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("[MapLink] Compatibility enabled - ignoring load from disk.");
			#endif

			SetDefaultValues();
			return;
		}

		if (!configName.Contains(".json"))
		{
			configName = configName + ".json";
		}

		if (FileExist(ModFolder + NestedFolder + DBFolder + "\\" + configName))
		{
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("Loading skills database from file: " + configName);
			#endif

			// If config exists, load file
			JsonFileLoader<ZenSkillsPlayerDB>.JsonLoadFile(ModFolder + NestedFolder + DBFolder + "\\" + configName, this);

			// If version mismatch, backup old version of json before replacing it
			if (CONFIG_VERSION != CURRENT_VERSION)
			{
				Error("CONFIG_VERSION " + CONFIG_VERSION + " != " + CURRENT_VERSION + " for player's skill DB: " + configName);
				JsonFileLoader<ZenSkillsPlayerDB>.JsonSaveFile(ModFolder + NestedFolder + DBFolder + "\\" + configName + "_old", this);
			}
			else
			{
				// Config file exists, was loaded successfully, and version matches - stop here.
				SetDefinitions();
				return;
			}
		}

		SetDefaultValues();
		Save(configName, "");
	}
	
	void SetDefaultValues()
	{
		CONFIG_VERSION = CURRENT_VERSION;

		foreach (string skillKey, ZenSkillDef skillDef : GetZenSkillsConfig().SkillDefs)
		{
			ZenSkill skill = new ZenSkill();
			
			foreach (string perkKey, ZenPerkDef perkDef : skillDef.Perks)
			{
				ZenPerk perk = new ZenPerk();
				skill.Perks.Set(perkKey, perk);
			}
			
			Skills.Set(skillKey, skill);
		}
		
		SetDefinitions(true);
	}
	
	// Server sets skill definitions, then syncs to clients on login.
	void SetDefinitions(bool firstInit = false)
	{
		foreach (string skillKey, ZenSkill skill : Skills)
		{
			skill.SetDef(GetZenSkillsConfig().SkillDefs.Get(skillKey));
			
			if (firstInit)
			{
				skill.EXP = skill.GetDef().StartingEXP;
			}
			
			foreach (string perkKey, ZenPerk perk : skill.Perks)
			{
				perk.SetDef(GetZenSkillsConfig().SkillDefs.Get(skillKey).Perks.Get(perkKey));
				
				// 10x rewards during debug mode.
				if (GetZenSkillsConfig().SharedConfig.DebugMode)
				{
					for (int i = 0; i < perk.GetDef().Rewards.Count(); i++)
					{
						perk.GetDef().Rewards.Set(i, Math.Clamp(perk.GetDef().Rewards.Get(i) * 10, 0, 99));
					}
				}
			}
		}
	}

	void Save(string playerID, string playerName)
	{
		if (GetGame().IsClient())
		{
			return;
		}
		
		// Don't save if ref to skills or perks has been lost (safeguard to premature garbage collection on shutdown etc)
		// this way worst-case scenario is a small rollback to last auto-save, versus total DB wipe if ref's are lost.

		if (GetZenSkillsConfig().ServerConfig.I_AM_USING_MAPLINK)
		{
			#ifdef ZENSKILLSDEBUG 
			Error("[MapLink] Compatibility enabled - ignoring save to disk.");
			#endif
			
			return;
		}
		
		if (!Skills)
		{
			#ifdef ZENSKILLSDEBUG
			Error("[ZenSkillsPlayerDB] NO SKILLS! For player " + playerName + " - " + playerID);
			#endif
			return;
		}
		
		if (!Skills.GetElement(0).Perks)
		{
			#ifdef ZENSKILLSDEBUG
			Error("[ZenSkillsPlayerDB] NO PERKS! For player " + playerName + " - " + playerID);
			#endif
			return; 
		}
		
		if (GetZenSkillsConfig().SharedConfig.AllowHighscores)
		{
			GetZenSkillsHighscoresDB().UpdateHighscores(playerID, playerName, Skills);
		}
		
		string configName = playerID;
		
		if (!configName.Contains(".json"))
		{
			configName = configName + ".json";
		}
			
		if (!FileExist(ModFolder))
		{
			MakeDirectory(ModFolder);
		}

		if (!FileExist(ModFolder + NestedFolder))
		{
			MakeDirectory(ModFolder + NestedFolder);
		}

		if (!FileExist(ModFolder + NestedFolder + DBFolder))
		{
			MakeDirectory(ModFolder + NestedFolder + DBFolder);
		}
		
		PlayerName = playerName;
		PlayerUID = playerID;

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkillsPlayerDB] Saving DB to file... " + configName);
		#endif

		JsonFileLoader<ZenSkillsPlayerDB>.JsonSaveFile(ModFolder + NestedFolder + DBFolder + "\\" + configName, this);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkillsPlayerDB] Successfully saved DB: " + configName + " for player " + playerName);
		#endif
	}

	int GetTierFromKey(string key)
	{
	    int tierIndex = key.IndexOf("_");
		
	    if (tierIndex < 0) 
			return 0;
		
	    string tier = key.Substring(0, tierIndex);
	    return tier.ToInt();
	}
	
	int CountUnlockedInTier(string skillKey, int checkTier)
	{
	    if (!Skills) 
			return 0;
		
	    ZenSkill skill = Skills.Get(skillKey);
	    if (!skill || !skill.Perks) 
			return 0;
	
	    int n = 0;
	    foreach (string key, ZenPerk perk : skill.Perks)
	    {
	        int tier = GetTierFromKey(key);
			
	        if (tier == checkTier && perk && perk.Level > 0) 
				n++;
	    }
		
	    return n;
	}
	
	bool CanUnlockPerk(string skillKey, string perkKey)
	{
	    if (!Skills) 
			return false;
		
	    ZenSkill skill = Skills.Get(skillKey);
	    if (!skill || !skill.Perks) 
			return false;
		
		ZenSkillDef skillDef = GetZenSkillsConfig().SkillDefs.Get(skillKey);
	    if (!skill || !skill.Perks) 
			return false;
	
	    ZenPerk targetPerk = skill.Perks.Get(perkKey);
	    if (!targetPerk) 
			return false;
		
		ZenPerkDef targetPerkDef = skillDef.Perks.Get(perkKey);
	    if (!targetPerkDef) 
			return false;
		
		if (IsPerkMaxed(targetPerk))
			return false;
		
		int availableSkillPoints = skill.EXP / skillDef.EXP_Per_Perk;
		if (availableSkillPoints <= 0)
			return false;
	
	    int tier = GetTierFromKey(perkKey);
	    if (tier <= 1) 
			return true;
	
	    int need = targetPerkDef.MinPerksRequiredFromPrevTier;
	    if (need <= 0) 
			return true;
	
	    int have = CountUnlockedInTier(skillKey, tier - 1);
	    return have >= need;
	}
	
	bool IsPerkMaxed(ZenPerk perk)
	{
	    if (!perk) 
			return true;
		
		ZenPerkDef perkDef = perk.GetDef();
		
	    if (!perkDef.Rewards) 
			return true;
		
	    int totalLevels = perkDef.Rewards.Count();
	    if (totalLevels <= 0) 
			return true;
		
	    if (perk.Level >= totalLevels) 
			return true;
		
	    return false;
	}
	
	int CountPerksUnlockedForSkill(string skillKey)
	{
		int perkCount = 0;
		
		foreach (string key, ZenPerk perk : Skills.Get(skillKey).Perks)
		{
			perkCount += perk.Level;
		}
		
		return perkCount;
	}
	
	int CountMissingPerksForUnlock(string skillKey, string perkKey)
	{
	    if (!Skills) 
			return 999;
		
	    ZenSkill skill = Skills.Get(skillKey);
	    if (!skill || !skill.Perks) 
			return 999;
	
	    ZenPerk target = skill.Perks.Get(perkKey);
	    if (!target) 
			return 999;
	
	    int tier = GetTierFromKey(perkKey);
	    if (tier <= 1) 
			return 0;
	
	    int need = target.GetDef().MinPerksRequiredFromPrevTier;
	    int have = CountUnlockedInTier(skillKey, tier - 1);
	    int missing = need - have;
		
	    if (missing > 0) 
			return missing;
		
	    return 0;
	}
	
	float GetPerkRewardPercent01(string skillKey, string perkKey)
	{
		ZenSkill skill = Skills.Get(skillKey);
		if (!skill)
		{
			Error("Invalid skillKey: " + skillKey);
			return 0;
		}
		
		ZenPerk perk = skill.Perks.Get(perkKey);
		if (!perk)
		{
			Error("Invalid perkKey: " + skillKey);
			return 0;
		}
		
		return perk.GetRewardPercent01();
	}
	
	float GetPerkReward(string skillKey, string perkKey)
	{
		ZenSkill skill = Skills.Get(skillKey);
		if (!skill)
		{
			Error("Invalid skillKey: " + skillKey);
			return 0;
		}
		
		ZenPerk perk = skill.Perks.Get(perkKey);
		if (!perk)
		{
			Error("Invalid perkKey: " + skillKey);
			return 0;
		}
		
		return perk.GetReward();
	}
	
	// Call from server on death, e.g. ApplyDeathExpPenalty(0.5) for 50%
	// Removes from unused EXP first, then from highest-tier perk levels.
	// If a remainder (< EXP_Per_Perk) remains, it deactivates one more highest-tier perk level
	// and refunds the overshoot back to unused EXP so the net loss equals the exact target.
	void ApplyDeathExpPenalty(float percent01)
	{
		if (PlayerUID == "")
			return;
		
	    if (percent01 <= 0)
	        return;
	
	    if (percent01 > 1)
	        percent01 = 1;
		
		#ifdef ZENSKILLSDEBUG 
		string debugMsg;
		#endif
	
	    foreach (string skillKey, ZenSkill skill : Skills)
	    {
	        if (!skill || !skill.Perks)
	            continue;
	
	        ZenSkillDef def = GetZenSkillsConfig().SkillDefs.Get(skillKey);
	        if (!def)
	            continue;
	
	        int expPerPerk = def.EXP_Per_Perk;
	
	        // Compute total allocated perk levels and total EXP (unused + allocated)
	        int unlockedPerkLevels = 0;
	        foreach (string perkKeyCount, ZenPerk perkCount : skill.Perks)
	        {
	            if (perkCount)
	            {
	                unlockedPerkLevels = unlockedPerkLevels + perkCount.Level;
	            }
	        }
	
	        int totalExp = skill.EXP + (unlockedPerkLevels * expPerPerk);
	        if (totalExp <= 0)
	            continue;
	
	        int targetLoss = Math.Floor(totalExp * percent01);
	        if (targetLoss <= 0)
	            continue;
	
	        // 1) Remove from unused EXP first
	        int removeFromUnused = 0;
	        if (skill.EXP >= targetLoss)
	        {
	            removeFromUnused = targetLoss;
	        }
	        else
	        {
	            removeFromUnused = skill.EXP;
	        }
	
	        skill.EXP = skill.EXP - removeFromUnused;
	        targetLoss = targetLoss - removeFromUnused;
	
	        if (targetLoss <= 0)
	            continue;
	
	        // 2) Determine max tier present
	        int maxTier = 0;
	        foreach (string perkKeyTierScan, ZenPerk perkTierScan : skill.Perks)
	        {
	            int tierScan = GetTierFromKey(perkKeyTierScan);
	            if (tierScan > maxTier)
	            {
	                maxTier = tierScan;
	            }
	        }
	
	        if (maxTier <= 0)
	            maxTier = 4;  // fallback if keys malformed
	
	        // 3) Remove whole perk levels from highest tier down
	        for (int tier = maxTier; tier >= 1; tier--)
	        {
	            if (targetLoss <= 0)
	                break;
	
	            foreach (string perkKey, ZenPerk perk : skill.Perks)
	            {
	                if (targetLoss <= 0)
	                    break;
	
	                if (!perk)
	                    continue;
	
	                if (GetTierFromKey(perkKey) != tier)
	                    continue;
	
	                while (perk.Level > 0 && targetLoss >= expPerPerk)
	                {
	                    perk.Level = perk.Level - 1;
	                    targetLoss = targetLoss - expPerPerk;
	                }
	            }
	        }
	
	        // 4) Exact remainder handling
	        // If targetLoss is now > 0 but < expPerPerk, drop one more highest-tier perk level
	        // and refund the overshoot back into unused EXP so that net loss equals the exact remainder.
	        if (targetLoss > 0)
	        {
	            int foundTier = -1;
	            string foundPerkKey = "";
	            ZenPerk foundPerk = null;
	
	            for (int tierFind = maxTier; tierFind >= 1; tierFind--)
	            {
	                foreach (string perkKeyFind, ZenPerk perkFind : skill.Perks)
	                {
	                    if (!perkFind)
	                        continue;
	
	                    if (GetTierFromKey(perkKeyFind) != tierFind)
	                        continue;
	
	                    if (perkFind.Level > 0)
	                    {
	                        foundTier = tierFind;
	                        foundPerkKey = perkKeyFind;
	                        foundPerk = perkFind;
	                        break;
	                    }
	                }
	
	                if (foundPerk)
	                    break;
	            }
	
	            if (foundPerk)
	            {
	                // Remove a level (cost = expPerPerk), then refund overshoot so net loss == targetLoss remainder
	                foundPerk.Level = foundPerk.Level - 1;
	
	                int refund = expPerPerk - targetLoss;  // strictly between 1 and expPerPerk - 1
	                if (refund > 0)
	                {
	                    skill.EXP = skill.EXP + refund;
	                }
	
	                targetLoss = 0;
	
	                #ifdef ZENSKILLSDEBUG
					debugMsg = "[DeathPenalty] skill=" + skillKey;
					debugMsg = debugMsg + " exact remainder handled by removing " + foundPerkKey;
					debugMsg = debugMsg + " and refunding " + refund + " EXP";
	                ZenSkillsPrint(debugMsg);
	                #endif
	            }
	            else
	            {
	                // No perk level left to drop; we cannot meet the exact remainder. Slight under-penalty remains.
	                #ifdef ZENSKILLSDEBUG
	                ZenSkillsPrint("[DeathPenalty] skill=" + skillKey + " no perk available for exact remainder");
	                #endif
	            }
	        }
	
	        // Ensure non-negative unused EXP
	        if (skill.EXP < 0)
	        {
	            skill.EXP = 0;
	        }
	
	        #ifdef ZENSKILLSDEBUG
	        int postUnlockedLevels = 0;
	        foreach (string pkPost, ZenPerk pPost : skill.Perks)
	        {
	            if (pPost)
	            {
	                postUnlockedLevels = postUnlockedLevels + pPost.Level;
	            }
	        }
	
	        int postTotalExp = skill.EXP + (postUnlockedLevels * expPerPerk);

			debugMsg = "[DeathPenalty] skill=" + skillKey;
			debugMsg = debugMsg + " expPerPerk=" + expPerPerk;
			debugMsg = debugMsg + " postUnusedEXP=" + skill.EXP;
			debugMsg = debugMsg + " postPerkLevels=" + postUnlockedLevels;
			debugMsg = debugMsg + " postTotalEXP=" + postTotalExp;
	        ZenSkillsPrint(debugMsg);
	        #endif
	    }
	}
}
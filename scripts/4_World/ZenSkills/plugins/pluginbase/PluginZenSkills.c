class PluginZenSkills extends PluginBase
{
	// playerUID, obj
	protected ref map<string, ref ZenSkillsPlayerDB> m_CachedPlayerDB;
	protected ref map<string, ref ZenSkillsExpNerf>  m_ExpNerfs;

	override void OnInit()
    {
        Print("[PluginZenSkills] :: OnInit.");
		
		// Runs on both client & server.
		m_CachedPlayerDB = new map<string, ref ZenSkillsPlayerDB>();
		RegisterRPCs();
		
		// Only server 
		if (GetGame().IsDedicatedServer())
		{
			if (GetZenSkillsEXP().ExpNerfConfig.EnableExpNerf)
			{
				m_ExpNerfs = new map<string, ref ZenSkillsExpNerf>();
			}
		}
    }

    override void OnDestroy()
	{
		Print("[PluginZenSkills] :: OnDestroy.");
		
		if (m_CachedPlayerDB)
			m_CachedPlayerDB.Clear();
		
		if (m_ExpNerfs)
			m_ExpNerfs.Clear();
    }
	
	// @CommunityFramework RPC registration
	void RegisterRPCs()
	{
		#ifdef SERVER
		// SERVER RECEIVE RPCs
        GetRPCManager().AddRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ServerReceive_PerkUnlock, this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ServerReceive_PerkReset, this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ServerReceive_HighscoreRequest, this, SingeplayerExecutionType.Server);
		#else 
		// CLIENT RECEIVE RPCs
        GetRPCManager().AddRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_ZenSkillsInit, this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_PerkUpdate, this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_ExpUpdate, this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_PerkReset, this, SingeplayerExecutionType.Client);
		GetRPCManager().AddRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_Highscores, this, SingeplayerExecutionType.Client);
		#endif
	}
	
	// Call this BEFORE awarding XP for an action
	void UpdateExpNerf(string playerUID, vector currentPos, string actionKey)
	{
		if (!m_ExpNerfs || !GetZenSkillsEXP().ExpNerfConfig.EnableExpNerf) 
			return;
	
		ZenSkillsExpNerf nerfDef = m_ExpNerfs.Get(playerUID);
		if (!nerfDef)
		{
			nerfDef = new ZenSkillsExpNerf();
			m_ExpNerfs.Set(playerUID, nerfDef);
		}
		
		nerfDef.Update(currentPos, actionKey);
	}
	
	// Ask for the multiplier to apply to the XP for THIS action
	float GetExpNerfFactor(string playerUID, string actionKey)
	{
		ZenSkillsExpNerf nerfDef = m_ExpNerfs.Get(playerUID);
		if (!nerfDef) 
			return 1;
		
		return nerfDef.GetCombinedFactor(actionKey);
	}

	ZenSkillsPlayerDB GetSkillsDB(string uid = "")
	{
		if (uid == "")
		{
			#ifndef SERVER
			PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
			if (pb && pb.GetIdentity())
			{
				uid = pb.GetIdentity().GetId();
			}
			#endif
		}
		
		if (uid == "")
			return null;

		return m_CachedPlayerDB.Get(uid);
	}
	
	void SetSkillsDB(string uid, ZenSkillsPlayerDB skillsDB)
	{
		m_CachedPlayerDB.Set(uid, skillsDB);

		DebugPrintSkillDB(uid);
	}
	
	ZenSkillsPlayerDB LoadZenSkillsDB(string uid)
	{
		if (GetGame().IsClient())
			return null;
		
		ZenSkillsPlayerDB skillsDB = GetSkillsDB(uid);
		if (skillsDB != null)
		{
			ZenSkillsPrint("[ZenSkills] Loading skill DB for: " + uid + " - from cache.");
			return skillsDB;
		}
		
		ZenSkillsPrint("[ZenSkills] Loading skill DB for: " + uid + " - from file.");

		skillsDB = new ZenSkillsPlayerDB();
		skillsDB.Load(uid);

		SetSkillsDB(uid, skillsDB);
		ZenSkillsPrint("[ZenSkills] Successfully loaded skill DB for: " + uid);
		return skillsDB;
	}
	
	void RemoveFromCache(string uid)
	{
		delete m_CachedPlayerDB.Get(uid);
	}
	
	void DebugPrintSkillDB(string uid)
	{
		if (!ZEN_SKILLS_DEBUG_ON)
			return;
		
		ZenSkillsPrint("[ZenSkills::DEBUG-PRINT] for uid: " + uid);
		
		foreach (string skillKey, ZenSkill skill : GetSkillsDB(uid).Skills)
		{
			ZenSkillsPrint("  [ZenSkills] Loaded skill: " + skillKey);
			
			foreach (string perkKey, ZenPerk perk : skill.Perks)
			{
				ZenSkillsPrint("    [ZenSkills] Loaded perk: " + perkKey + " Level=" + perk.Level);
			}
		}
	}
	
	void DebugPrintSkillDB()
	{
		if (!ZEN_SKILLS_DEBUG_ON)
			return;
		
		for (int i = 0; i < m_CachedPlayerDB.Count(); i++)
		{
			string uid = m_CachedPlayerDB.GetKey(i);
			ZenSkillsPrint("[ZenSkills::DEBUG-PRINT] for uid: " + uid);
			
			foreach (string skillKey, ZenSkill skill : GetSkillsDB(uid).Skills)
			{
				ZenSkillsPrint("  [ZenSkills] Loaded skill: " + skillKey);
				
				foreach (string perkKey, ZenPerk perk : skill.Perks)
				{
					ZenSkillsPrint("    [ZenSkills] Loaded perk: " + perkKey + " Level=" + perk.Level);
				}
			}
		}
	}
	
	void AddEXP_Action(PlayerBase player, string actionKey, float modifier = 1, bool forceRawEXP = false)
	{
		if (!player || !player.GetIdentity())
			return;
		
		#ifdef EXPANSIONMODAI
		if (player.IsAI())
			return;
		#endif
		
		if (actionKey == "")
			return;

		actionKey.ToLower();
		
		//! NOTE: This approach allows EXP to be awarded for multiple skills per action.
		foreach (string skillKey, ZenSkillsEXPDefHolder expDefHolder : GetZenSkillsEXP().ExpDefs)
		{
			foreach (string actionKeyDef, ZenSkillsEXPDef expDef : expDefHolder.ExpDefs)
			{
				actionKeyDef.ToLower();
				
				if (actionKey == actionKeyDef)
				{
					bool dontApplyNerf = !expDef.ApplyNerf || forceRawEXP;
					bool dontAllowBoosts = !expDef.ApplyBoosts || forceRawEXP;
					int exp = expDef.EXP;
					int expAdjusted = exp;
					
					if (expDef.AllowModifier && !forceRawEXP)
					{
						expAdjusted = Math.Floor(exp * modifier);
						if (expAdjusted <= 0)
							expAdjusted = 1;
					}
					
					#ifdef ZENSKILLSDEBUG 
					string debugMsg = "EXP=" + exp;
					debugMsg = debugMsg + " EXP_Adjusted=" + expAdjusted;
					debugMsg = debugMsg + " modifier=" + modifier;
					debugMsg = debugMsg + "x for skill=" + skillKey;
					debugMsg = debugMsg + " actionKey=" + actionKey;
					debugMsg = debugMsg + " dontApplyNerf=" + dontApplyNerf;
					ZenSkillsPrint(debugMsg);
					#endif
					
					if (expAdjusted > 0)
					{
						AddEXP(player, skillKey, expAdjusted, actionKey, dontApplyNerf, dontAllowBoosts);
					}
				}
			}
		}
	}
	
	void AddEXP(notnull PlayerBase player, string skillKey, int origEXP, string actionKey = "", bool dontApplyNerf = false, bool dontApplyBoosts = false)
	{
	    #ifdef EXPANSIONMODAI
	    if (player.IsAI())
	        return;
	    #endif
	
	    string uid = player.GetIdentity().GetId();
	
	    ZenSkill skill = GetSkillsDB(uid).Skills.Get(skillKey);
	    if (!skill)
	    {
	        Error("This skill does not exist - typo?: " + skillKey);
	        return;
	    }
	
	    int expPerPerk      = skill.GetDef().EXP_Per_Perk;
	    int maxPerks        = skill.GetDef().MaxAllowedPerks;
	    int maxAllowedEXP   = maxPerks * expPerPerk;
	
	    int unlockedPerks   = skill.CountPerksUnlocked();
	    int unusedPerks     = skill.EXP / expPerPerk;
	    int totalPerks      = unlockedPerks + unusedPerks;
	
	    int usedEXPByPerks  = unlockedPerks * expPerPerk;
	    int currentTotalEXP = usedEXPByPerks + skill.EXP;
	
	    // Hard stop if already at or above perk cap
	    if (totalPerks >= maxPerks)
	    {
	        #ifdef ZENSKILLSDEBUG
	        ZenSkillsPrint("Cannot add EXP to " + skillKey + " for " + uid + ": perkcount=" + totalPerks + "/" + maxPerks);
	        #endif
	        return;
	    }
	
	    // Compute remaining room under the absolute EXP cap
	    int remainingEXP = maxAllowedEXP - currentTotalEXP;
	    if (remainingEXP <= 0)
	        return;
	
	    // ----- Multipliers and nerfs -----
	    float perkCountNerfFactor = 1.0;
	    if (!dontApplyNerf && skill.GetDef().EXP_Perk_Modifier > 0 && totalPerks > 0)
	    {
	        perkCountNerfFactor = 1.0 - (totalPerks * skill.GetDef().EXP_Perk_Modifier);
	        perkCountNerfFactor = Math.Clamp(perkCountNerfFactor, 0.0, 1.0);
	    }
	
	    float campNerfFactor = 1.0;
	    if (!dontApplyNerf && actionKey != "" && GetZenSkillsEXP().ExpNerfConfig.EnableExpNerf)
	    {
	        campNerfFactor = GetExpNerfFactor(uid, actionKey);
	        campNerfFactor = Math.Clamp(campNerfFactor, 0.0, 1.0);
	    }
	
	    float xpBoostInjection = 1.0;
	    if (!dontApplyBoosts && player.GetModifiersManager().IsModifierActive(ZenSkillsModifiers.MDF_EXPBOOST))
	    {
	        xpBoostInjection = GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostMulti;
	    }
	
	    float xpBoostPerk = 1.0;
	    ZenPerk perkNode = null;
	    if (!dontApplyBoosts && skill.Perks && skill.Perks.Find("1_1", perkNode) && perkNode)
	    {
	        xpBoostPerk = 1.0 + perkNode.GetRewardPercent01();
	    }
	
	    // Final award = base * boosts * nerfs
		float rawAward;
		if (dontApplyBoosts)
		{
			rawAward = origEXP * GetZenSkillsEXP().ExpNerfConfig.GlobalExpMultiplier;
		} else 
		{
			rawAward = origEXP * xpBoostPerk * xpBoostInjection * campNerfFactor * perkCountNerfFactor * GetZenSkillsEXP().ExpNerfConfig.GlobalExpMultiplier;
		}

	    int finalAwardEXP = Math.Round(rawAward);
	
	    // Clamp to remaining cap; do NOT force at least 1 if no room remains
	    finalAwardEXP = Math.Clamp(finalAwardEXP, 0, remainingEXP);
	
	    if (finalAwardEXP <= 0)
	    {
	        #ifdef ZENSKILLSDEBUG
	        ZenSkillsPrint("No EXP awarded to " + skillKey + " for " + uid + " (award <= 0 or cap reached)");
	        #endif
	        return;
	    }
	
	    // Apply xp
	    skill.EXP += finalAwardEXP;
	
	    if (actionKey != "" && GetZenSkillsEXP().ExpNerfConfig.EnableExpNerf)
	    {
	        UpdateExpNerf(uid, player.GetPosition(), actionKey);
	    }
	
		if (GetZenSkillsAnalyticsPlugin())
		{
			GetZenSkillsAnalyticsPlugin().OnExpAdded(skillKey, finalAwardEXP, uid, actionKey);
		}
		
		#ifdef ZENSKILLSDEBUG 
		string debugMsg = "EXP boost multi for skill=" + skillKey;
		debugMsg = debugMsg + " actionKey=" + actionKey;
		debugMsg = debugMsg + " origEXP=" + origEXP;
		debugMsg = debugMsg + " xpBoostPerk=" + xpBoostPerk + "x";
		debugMsg = debugMsg + " xpBoostInjection=" + xpBoostInjection + "x";
		debugMsg = debugMsg + " campNerfFactor=" + campNerfFactor + "x";
		debugMsg = debugMsg + " perkCountNerfFactor=" + perkCountNerfFactor + "x";
		debugMsg = debugMsg + " globalExpModifier=" + GetZenSkillsEXP().ExpNerfConfig.GlobalExpMultiplier + "x";
		debugMsg = debugMsg + " finalAwardEXP=" + finalAwardEXP;
		debugMsg = debugMsg + " for " + uid;
		ZenSkillsPrint(debugMsg);
		#endif
		
		GetRPCManager().SendRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_ExpUpdate, new Param2<string, int>(skillKey, skill.EXP), true, player.GetIdentity());
	}
	
	void ResetPerks(PlayerIdentity playerIdentity, array<string> selectedSkills, bool applyCost = true)
	{
		string uid = playerIdentity.GetId();
		ZenSkillsPlayerDB db = GetSkillsDB(uid);
		
		if (!db || !playerIdentity)
		{
			Error("Failed to get skills database for " + uid);
			return;
		}

		foreach (string selectedSkill : selectedSkills)
		{
			ZenSkill skill = db.Skills.Get(selectedSkill);
			if (!skill)
			{
				Error("Skill key doesn't exist: " + selectedSkill);
				return;
			}
	
			int perkRefund;
			int totalPerksLeft;
			int refundedTotalEXP = GetResultingRefundPerksEXP(skill, perkRefund, totalPerksLeft, applyCost);
			skill.EXP = refundedTotalEXP;
			
			foreach (string key, ZenPerk perk : skill.Perks)
			{
				perk.Level = 0;
			}
			
			Print("[ZenSkills] " + playerIdentity.GetId() + " refunded all their perks and received " + refundedTotalEXP + " EXP back for skill " + selectedSkill);
		}

		SavePlayerDB(db, playerIdentity);
		ResyncToClientDB(playerIdentity, db);
	}

	void SavePlayerDB(notnull ZenSkillsPlayerDB db, notnull PlayerIdentity id)
	{
		db.Save(id.GetId(), id.GetName());
	}
	
	void ResyncToClientDB(PlayerIdentity identity, ZenSkillsPlayerDB db)
	{
		GetRPCManager().SendRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_PerkReset, new Param1<ref ZenSkillsPlayerDB>(db), true, identity);
	}

	void SendLoginInitClientDB(PlayerIdentity identity, ZenSkillsPlayerDB db)
	{
		GetRPCManager().SendRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_ZenSkillsInit, new Param3<ref ZenSkillsSharedConfig, ref map<string, ref ZenSkillDef>, ref ZenSkillsPlayerDB>(GetZenSkillsConfig().SharedConfig, GetZenSkillsConfig().SkillDefs, db), true, identity);
	}
	
	int GetResultingRefundPerksEXP(notnull ZenSkill skill, out int perkRefund, out int totalPerksLeft, bool applyCost = true)
	{
		int skillEXP = skill.CountPerksUnlocked() * skill.GetDef().EXP_Per_Perk;
		
		int resultingEXP = skillEXP;
		if (applyCost)
		{
			resultingEXP = Math.Max(0, (int)(skillEXP * skill.GetDef().EXP_Refund_Modifier));
		}
		
		perkRefund = Math.Max(0, (int)(resultingEXP / skill.GetDef().EXP_Per_Perk));
		totalPerksLeft = perkRefund + skill.CountPerksUnused();
		
		// Return perk refund EXP + any unused EXP (EXP is deducted whenever a perk is unlocked)
		return resultingEXP + skill.EXP;
	}
	
	void UpdateClientGUI(int soundID)
	{
		if (!GetGame().IsClient())
			return;
		
		if (!GetGame().GetUIManager().GetMenu())
			return;
		
		ZenSkillsGUI gui = ZenSkillsGUI.Cast(GetGame().GetUIManager().GetMenu());
		if (gui)
		{
			gui.ForceUpdateFromServer(soundID);
		}
	}
	
	//!================================ RPCs ================================!
	//========================================================================
	
	//! Server -> client
	
    void RPC_ClientReceive_ZenSkillsInit(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        Param3<ref ZenSkillsSharedConfig, ref map<string, ref ZenSkillDef>, ref ZenSkillsPlayerDB> data;
        if (!ctx.Read(data))
        {
            Error("Error sync'ing server-side data to client - RPC_ClientReceive_ZenSkillsInit");
            return;
        }

		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetIdentity())
			return;
		
		GetZenSkillsConfig().SharedConfig = data.param1;
		map<string, ref ZenSkillDef> skillDefs = data.param2;
		ZenSkillsPlayerDB receivedDB = data.param3;
		ZenSkillsPlayerDB clientDB = new ZenSkillsPlayerDB();
		
		// Plugin starts before this is received from server, so set it for client now.
		ZEN_SKILLS_DEBUG_ON = GetZenSkillsConfig().SharedConfig.DebugMode;
		
		// I reconstruct the map<> because RPC does some serious fuckery with maps. Arrays work with direct copy, maps don't.
		// When you send a map over RPC and copy it directly it seems to get garbage collected no matter what, unlike arrays.
		// Rebuilding the map using the data received from RPC while it's still in memory seems to hold just fine.
		// Maybe I'm just retarded and doing it wrong - in fact that's the most likely cause - but whatever, this works.

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("------ PRINTING SKILL DEFS :: INIT ------");
		#endif
		
		GetZenSkillsConfig().SkillDefs.Clear();
		foreach (string skillKey, ZenSkillDef rpcSkillDef : skillDefs)
		{
			ZenSkillDef skillDef = new ZenSkillDef();
			skillDef.CopySkillDef(rpcSkillDef);
			GetZenSkillsConfig().SkillDefs.Set(skillKey, skillDef);
		}
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("------ PRINTING PLAYER SKILLS :: INIT ------");
		ZenSkillsPrint("SkillCount: " + receivedDB.Skills.Count());
		#endif
		
		clientDB.Skills.Clear();
		foreach (string playerSkillKey, ZenSkill rpcPlayerSkill : receivedDB.Skills)
		{
			ZenSkill skill = new ZenSkill(rpcPlayerSkill.EXP);
			skill.SetDef(GetZenSkillsConfig().SkillDefs.Get(playerSkillKey));
			
			foreach (string playerPerkKey, ZenPerk playerPerk : rpcPlayerSkill.Perks)
			{
				ZenPerk perk = new ZenPerk(playerPerk.Level);
				perk.SetDef(GetZenSkillsConfig().SkillDefs.Get(playerSkillKey).Perks.Get(playerPerkKey));
				skill.Perks.Set(playerPerkKey, perk);
				
				#ifdef ZENSKILLSDEBUG
				ZenSkillsPrint("[ZenSkill] Received perk level for skill=" + playerSkillKey + " perk=" + playerPerkKey + " level=" + perk.Level); 
				ZenSkillsPrint("           RewardCount=" + perk.GetDef().Rewards.Count());
				foreach (float rewardValue : perk.GetDef().Rewards)
				{
					ZenSkillsPrint("             RewardValue=" + rewardValue);
				}
				#endif
			}
			
			clientDB.Skills.Set(playerSkillKey, skill);
		}

        SetSkillsDB(pb.GetIdentity().GetId(), clientDB);
		ZenSkillsPlayerDB.RECEIVED_DATA = true;
    }
	
	void RPC_ClientReceive_PerkReset(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        Param1<ref ZenSkillsPlayerDB> data;
        if (!ctx.Read(data))
        {
            Error("Error sync'ing server-side data to client - RPC_ClientReceive_FullUpdate");
            return;
        }

		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetIdentity())
			return;

		ZenSkillsPlayerDB receivedDB = data.param1;
		ZenSkillsPlayerDB clientDB = GetSkillsDB();
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("------ PRINTING PLAYER SKILLS: FULL UPDATE ------");
		ZenSkillsPrint("SkillCount: " + receivedDB.Skills.Count());
		#endif
		
		foreach (string playerSkillKey, ZenSkill rpcPlayerSkill : receivedDB.Skills)
		{
			ZenSkill skill = new ZenSkill(rpcPlayerSkill.EXP);
			skill.SetDef(GetZenSkillsConfig().SkillDefs.Get(playerSkillKey));
			
			foreach (string playerPerkKey, ZenPerk playerPerk : rpcPlayerSkill.Perks)
			{
				ZenPerk perk = new ZenPerk(playerPerk.Level);
				perk.SetDef(GetZenSkillsConfig().SkillDefs.Get(playerSkillKey).Perks.Get(playerPerkKey));
				skill.Perks.Set(playerPerkKey, perk);
				
				#ifdef ZENSKILLSDEBUG
				ZenSkillsPrint("[ZenSkill] Received perk level for skill=" + playerSkillKey + " perk=" + playerPerkKey + " level=" + perk.Level); 
				ZenSkillsPrint("           RewardCount=" + perk.GetDef().Rewards.Count());
				foreach (float rewardValue : perk.GetDef().Rewards)
				{
					ZenSkillsPrint("             RewardValue=" + rewardValue);
				}
				#endif
			}
			
			clientDB.Skills.Set(playerSkillKey, skill);
		}

        SetSkillsDB(pb.GetIdentity().GetId(), clientDB);
		UpdateClientGUI(ZenSkillConstants.SOUND_PERKSRESET);
    }	

	void RPC_ClientReceive_ExpUpdate(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
		Param2<string, int> data;
        if (!ctx.Read(data))
        {
            Error("Error sync'ing server-side data to client - RPC_ClientReceive_ExpUpdate");
            return;
        }
		
		ZenSkillsPlayerDB db = GetSkillsDB();
		
		if (!db)
		{
			Error("Failed to get skills database");
			return;
		}
		
		string selectedSkill = data.param1;
		ZenSkill skill = db.Skills.Get(selectedSkill);
		if (!skill)
		{
			Error("Failed to get skill entry from DB for skill=" + selectedSkill);
			return;
		}
		
		int previousExp = skill.EXP;
		skill.EXP = data.param2;
		int difference = skill.EXP - previousExp;
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint(selectedSkill + " EXP Gained: " + difference);
		#endif
		
		Hud hud = GetGame().GetMission().GetHud();

		if (hud && hud.GetZenSkillsHUD())
		{
			ZenSkillsHUD skillHUD = ZenSkillsHUD.Cast(hud.GetZenSkillsHUD());
			string skillString = "#STR_ZenSkills_Name_" + ZenSkillFunctions.FirstLetterUppercase(selectedSkill);
			
			skillHUD.SetExpGainedLabel(skill.ProgressToNextPerk(), skillString + " #STR_ZenSkills_GUI_ExpGained: +" + difference);
			
			int perkCount = Math.Floor(skill.EXP / skill.GetDef().EXP_Per_Perk);
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("Perks Unlocked: " + perkCount);
			#endif
			
			if (perkCount > 0)
			{
				string suffix = "#STR_ZenSkills_GUI_Perk";
				if (perkCount > 1)
					suffix = "#STR_ZenSkills_GUI_Perks";
				
				skillHUD.SetPerkUnlockedLabel(ZenSkillFunctions.FirstLetterUppercase(selectedSkill), perkCount, "#STR_ZenSkills_GUI_Unlocked " + perkCount + " " + skillString + " " + suffix);
			}
		}
    }
	
	void RPC_ClientReceive_PerkUpdate(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
		Param4<string, string, int, int> data;
        if (!ctx.Read(data))
        {
            Error("Error sync'ing server-side data to client - RPC_ClientReceive_PerkUpdate");
            return;
        }
		
		ZenSkillsPlayerDB db = GetSkillsDB();
		
		if (!db)
		{
			Error("Failed to get skills database");
			return;
		}
		
		string selectedSkill = data.param1;
		string selectedPerk = data.param2;
		int newPerkLevel = data.param3;
		int newExpValue = data.param4;
		
		ZenSkill skill = db.Skills.Get(selectedSkill);
		if (!skill)
		{
			Error("Failed to get skill entry from DB for skill=" + selectedSkill);
			return;
		}
		
		ZenPerk perk = skill.Perks.Get(selectedPerk);
		if (!perk)
		{
			Error("Failed to get perk from DB for skill=" + selectedSkill + " perk=" + selectedPerk);
			return;
		}
		
		int expPerPerk = skill.GetDef().EXP_Per_Perk;
		if (skill.EXP < expPerPerk)
		{
			Error("Player does not have enough EXP for the requested perk skill=" + selectedSkill + " perk=" + selectedPerk + " unlock=" + skill.EXP + " < " + expPerPerk);
			return;
		}

		perk.Level = newPerkLevel;
		skill.EXP = newExpValue;
		
		int soundID = ZenSkillConstants.SOUND_PERKUNLOCKED;
		if (perk.Level == perk.GetDef().Rewards.Count())
			soundID = ZenSkillConstants.SOUND_PERKUNLOCKED_FINAL;
		
		UpdateClientGUI(soundID);
    }
	
	void RPC_ClientReceive_Highscores(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
		Param1<ref ZenSkillsHighscoresDB> data;
        if (!ctx.Read(data) || !data.param1)
        {
            Error("Error sync'ing server-side data to client - RPC_ClientReceive_Highscores");
            return;
        }
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("RPC_ClientReceive_Highscores - copying data received from server.");
		#endif
		
		map<string, ref array<ref ZenSkillsHighscoreDef>> receivedHighscores = data.param1.Highscores;
		GetZenSkillsHighscoresDB().MaxEntries = data.param1.MaxEntries;
		GetZenSkillsHighscoresDB().Highscores.Clear();
		
		foreach (string skillKey, array<ref ZenSkillsHighscoreDef> arrayScores : receivedHighscores)
		{
			array<ref ZenSkillsHighscoreDef> newArrayScores = new array<ref ZenSkillsHighscoreDef>();
			
			foreach (ZenSkillsHighscoreDef hsDef : arrayScores)
			{
				ZenSkillsHighscoreDef newDef = new ZenSkillsHighscoreDef();
				newDef.PlayerName = hsDef.PlayerName;
				newDef.Level = hsDef.Level;
				newArrayScores.Insert(newDef);
				
				#ifdef ZENSKILLSDEBUG 
				ZenSkillsPrint("  Received " + newDef.PlayerName + " level: " + newDef.Level);
				#endif
			}
			
			GetZenSkillsHighscoresDB().Highscores.Set(skillKey, newArrayScores);
		}
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Received " + GetZenSkillsHighscoresDB().Highscores.Count() + " total highscores entries.");
		#endif
		
		ZenSkillsHighscores gui = ZenSkillsHighscores.Cast(GetGame().GetUIManager().GetMenu());
		if (gui)
		{
			gui.ForceUpdateFromServer();
		}
    }
	
	//! Client -> server
	
	void RPC_ServerReceive_PerkUnlock(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
		if (!sender)
			return;
		
        Param2<string, string> data;
        if (!ctx.Read(data))
        {
            Error("Error sync'ing server-side data to client - RPC_ServerReceive_PerkUnlock");
            return;
        }

		string uid = sender.GetId();
		ZenSkillsPlayerDB db = GetSkillsDB(uid);
		
		if (!db)
		{
			Error("Failed to get skills database for " + uid);
			return;
		}
		
		string selectedSkill = data.param1;
		string selectedPerk = data.param2;
		
		if (!db.CanUnlockPerk(selectedSkill, selectedPerk))
		{
			NotificationSystem.SendNotificationToPlayerIdentityExtended(sender, 10, "SKILLS", "You requested to unlock skill=" + selectedSkill + " perk=" + selectedPerk);
			return;
		}
		
		ZenSkill skill = db.Skills.Get(selectedSkill);
		if (!skill)
		{
			Error("Failed to get skill entry from DB for skill=" + selectedSkill);
			return;
		}
		
		ZenPerk perk = skill.Perks.Get(selectedPerk);
		if (!perk)
		{
			Error("Failed to get perk from DB for skill=" + selectedSkill + " perk=" + selectedPerk);
			return;
		}
		
		int expPerPerk = skill.GetDef().EXP_Per_Perk;
		if (skill.EXP < expPerPerk)
		{
			Error("Player does not have enough EXP for the requested perk skill=" + selectedSkill + " perk=" + selectedPerk + " unlock=" + skill.EXP + " < " + expPerPerk);
			return;
		}
		
		perk.Level += 1;
		skill.EXP -= expPerPerk;
		
		SavePlayerDB(db, sender);
		
		GetRPCManager().SendRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_PerkUpdate, new Param4<string, string, int, int>(selectedSkill, selectedPerk, perk.Level, skill.EXP), true, sender);
    }
	
	void RPC_ServerReceive_PerkReset(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
		if (!sender)
			return;
		
        Param1<string> data;
        if (!ctx.Read(data))
        {
            Error("Error sync'ing server-side data to client - RPC_ServerReceive_PerkReset");
            return;
        }

		string selectedSkill = data.param1;
		array<string> selectedSkills = new array<string>();
		selectedSkills.Insert(selectedSkill);
		
		ResetPerks(sender, selectedSkills);
    }
	
	void RPC_ServerReceive_HighscoreRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
		if (!sender)
			return;
		
        Param1<bool> data;
        if (!ctx.Read(data))
        {
            Error("Error sync'ing server-side data to client - RPC_ServerReceive_HighscoreRequest");
            return;
        }
		
		GetRPCManager().SendRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ClientReceive_Highscores, new Param1<ref ZenSkillsHighscoresDB>(GetZenSkillsHighscoresDB()), true, sender);
    }
}

static ref PluginZenSkills m_PluginZenSkills;

static PluginZenSkills GetZenSkillsPlugin()
{
	if (!m_PluginZenSkills)
	{
		m_PluginZenSkills = PluginZenSkills.Cast(GetPlugin(PluginZenSkills));
	}
	
	return m_PluginZenSkills;
}
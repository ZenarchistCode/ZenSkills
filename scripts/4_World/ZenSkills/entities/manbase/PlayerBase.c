modded class PlayerBase
{
	protected ref ZenSkillsPlayerDB m_ZenSkillsDB;
	protected ref Timer m_ZenSkillsTimer;
	
	void StartZenSkillsSaveTimer()
	{
		// I do this here instead of mass-saving all DBs in the plugin so it staggers the file-writing.
		if (GetGame().IsDedicatedServer() && !m_ZenSkillsTimer)
		{
			m_ZenSkillsTimer = new Timer();
			m_ZenSkillsTimer.Run(GetZenSkillsConfig().ServerConfig.AutoSaveTimerSecs, this, "SaveZenSkillsDB", null, true);
		}
	}
	
	void SaveZenSkillsDB()
	{
		if (GetGame().IsDedicatedServer() && GetIdentity() && !GetZenSkillsConfig().ServerConfig.I_AM_USING_MAPLINK)
		{
			GetZenSkillsDB().Save(GetIdentity().GetId(), GetZenSkillsName());
		}
	}
	
	// Override this for mods that change player name in other ways (eg. Syberia Project)
	string GetZenSkillsName()
	{
		return GetIdentity().GetName();
	}

	ZenSkillsPlayerDB GetZenSkillsDB()
	{
		if (!m_ZenSkillsDB)
		{
			if (!GetIdentity())
			{
				return null;
			}
			
			m_ZenSkillsDB = GetZenSkillsPlugin().GetSkillsDB(GetIdentity().GetId());
		}
		
		return m_ZenSkillsDB;
	}

	void SetZenSkillsDB(ZenSkillsPlayerDB db)
	{
		m_ZenSkillsDB = db;
		GetZenSkillsPlugin().SetSkillsDB(GetIdentity().GetId(), m_ZenSkillsDB);
		GetZenSkillsPlugin().SendLoginInitClientDB(GetIdentity(), m_ZenSkillsDB);
	}
	
	void AddZenSkillEXP(string skillKey, int exp)
	{
		#ifndef SERVER
		Error("Tried to call AddZenSkillEXP() on client! Make sure the function you're doing this in runs on the server and add a IsServer-check.");
		return;
		#endif
		
		GetZenSkillsPlugin().AddEXP(this, skillKey, exp);
	}
	
	void AddZenSkillEXP_Action(string actionKey, int exp)
	{
		#ifndef SERVER
		Error("Tried to call AddZenSkillEXP_Action() on client! Make sure the function you're doing this in runs on the server and add a IsServer-check.");
		return;
		#endif
		
		GetZenSkillsPlugin().AddEXP_Action(this, actionKey, exp);
	}
	
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);
		
		if (rpc_type == ZenSkillConstants.RPC_ZenExpBoostNotify)
		{
			Param2<bool, int> data = new Param2<bool, int>(false, 0); 
			if (ctx.Read(data))
			{
				ZenSkillsGUI.m_ZenSkillsExpBoostActive = data.param1;
				ZenSkillsGUI.m_ZenSkillsExpBoostLeftSecs = data.param2;
				ZenSkillsGUI.m_ZenSkillsExpReceivedTimestamp = GetGame().GetTime();
			}
			
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("ExpBoostActive=" + ZenSkillsGUI.m_ZenSkillsExpBoostActive + " m_ZenSkillsExpBoostLeftSecs=" + ZenSkillsGUI.m_ZenSkillsExpBoostLeftSecs);
			#endif
		}
	}
	
	override void OnDisconnect()
	{
		super.OnDisconnect();
		
		if (GetGame().IsClient())
			return;
		
		Print("[ZenSkills] Saving skill DB to file for player: " + GetCachedID());
		SaveZenSkillsDB();
		
		if (!GetZenSkillsConfig().ServerConfig.EnableDatabaseCache)
		{
			GetZenSkillsPlugin().RemoveFromCache(GetCachedID());
		}
	}
	
	override void ProcessFeetDamageServer(int pUserInt)
	{
		#ifdef EXPANSIONMODAI
		if (IsAI())
		{
			super.ProcessFeetDamageServer(pUserInt);
			return;
		}
		#endif

		// 1) snapshot shoes + health
		EntityAI shoes = GetInventory().FindAttachment(InventorySlots.FEET);

		float hpBefore = 0.0;
		if (shoes)
		{
			hpBefore = shoes.GetHealth("", "");
		}

		// 2) vanilla logic
		super.ProcessFeetDamageServer(pUserInt);

		// 3) refund part of the damage after vanilla
		if (!shoes)
		{
			return;
		}

		float hpAfter = shoes.GetHealth("", "");
		float delta = hpBefore - hpAfter; // positive if damaged
		if (delta <= 0)
		{
			return;
		}

		float pct = GetZenPerkRewardPercent01("survival", ZenPerks.SURVIVAL_SHOES_DURABILITY);
		if (pct <= 0)
		{
			return;
		}

		// don't resurrect ruined shoes
		if (shoes.GetHealthLevel() >= GameConstants.STATE_RUINED)
		{
			return;
		}

		float refund = delta * pct;
		float maxHP = shoes.GetMaxHealth("", "");
		float space = maxHP - hpAfter;
		if (refund > space)
		{
			// Clamp to max health
			refund = space;
		}

		if (refund > 0)
		{
			shoes.AddHealth("", "", refund);
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("SHOE PERK: Refund hp=" + refund + " hpBefore=" + hpBefore + " delta=" + delta + " hpNow=" + shoes.GetHealth() + " wouldBeHP=" + (shoes.GetHealth() + refund));
			#endif
		}
	}
	
	override float GetImmunity()
	{
		#ifdef EXPANSIONMODAI
		if (IsAI())
			return super.GetImmunity();
		#endif

		float immunity = super.GetImmunity();
		float modifier = 1 + GetZenPerkRewardPercent01("survival", ZenPerks.SURVIVAL_IMMUNITY_BOOST);
		float boostedImmunity = Math.Clamp(immunity * modifier, 0, 1);
		
		#ifdef ZENSKILLSDEBUG
		if (modifier > 1)
		{
			ZenSkillsPrint("Immunity modifier perk: " + modifier + " oldImmunity=" + immunity + " boosted=" + boostedImmunity);
		}
		#endif
		
		return boostedImmunity;
	}
	
	float GetZenPerkRewardPercent01(string skillKey, string perkKey)
	{
		#ifdef EXPANSIONMODAI
		if (IsAI())
			return 1;
		#endif
		
		ZenSkillsPlayerDB db = GetZenSkillsDB();
		if (!db)
		{
			return 0;
		}

		return db.GetPerkRewardPercent01(skillKey, perkKey);
	}
	
	float GetZenPerkReward(string skillKey, string perkKey)
	{
		#ifdef EXPANSIONMODAI
		if (IsAI())
			return 0;
		#endif
		
		ZenSkillsPlayerDB db = GetZenSkillsDB();
		if (!db)
		{
			return 0;
		}

		return db.GetPerkReward(skillKey, perkKey);
	}
	
	override void OnJumpStart()
	{
		super.OnJumpStart();
		
		if (!ZEN_SKILLS_DEBUG_ON)
			return;
		
		if (!GetGame().IsDedicatedServer())
			return;

		AddZenSkillEXP(GetZenSkillsConfig().SkillDefs.GetKeyArray().GetRandomElement(), GetZenSkillsConfig().ServerConfig.DebugJumpEXP);
	}
	
	override void SetActions()
	{
		super.SetActions();

		AddAction(ActionZenSkillsNurturePlant);
	}
	
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);
		
		ZenSkillFunctions.HandleEntityKilledEXP(this, killer);
	}
}
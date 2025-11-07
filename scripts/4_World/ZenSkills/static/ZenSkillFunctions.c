class ZenSkillFunctions
{
	static string FirstLetterUppercase(string text)
	{
		string firstLetterUpper = text.Substring(0, 1);
		firstLetterUpper.ToUpper();
		return firstLetterUpper + text.Substring(1, text.Length() - 1);
	}
	
	static string GetInputKeyText(string actionInputName)
	{
		string keyName = "<#STR_ZenSkills_GUI_UnsetKey>";
		UAInput input = GetUApi().GetInputByName("UAZenSkillsOpen");
		
		if (!input)
			return keyName;
		
		input.SelectAlternative(0); // select primary bind
		for (int i = 0; i < input.BindKeyCount(); i++ )
		{
		  	int keyIndex = input.GetBindKey(0);
		  	keyName = GetUApi().GetButtonName(keyIndex);
		}
		
		return keyName;
	}
	
	//! Debug message - sends a server-side player message to all online players
	static void SendGlobalMessage(string msg)
	{
		SendGlobalMessageEx(msg, "[SERVER] ");
	}

	//! Debug message - sends a server-side player message to all online players
	static void SendGlobalMessageEx(string msg, string prefix = "")
	{
		#ifdef SERVER
		array<Man> players = new array<Man>;
		GetGame().GetWorld().GetPlayerList(players);
		for (int x = 0; x < players.Count(); x++)
		{
			PlayerBase pb = PlayerBase.Cast(players.Get(x));
			if (pb)
			{
				SendPlayerMessage(pb, prefix + msg);
			}
		}
		#endif
	}

	//! Display client message ONLY on client
	static void ZenClientMessage(string message)
	{
#ifndef SERVER
		if (GetGame().GetPlayer())
		{
			GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", message, ""));
		}
#endif
	}

	//! Print a debug chat message both client-side & server-side
	static void DebugMessage(string message)
	{
		ZenClientMessage("[CLIENT] " + message);
		SendGlobalMessage(message);
	}

	//! Send a message to the given player
	static void SendPlayerMessage(PlayerBase player, string msg)
	{
#ifdef SERVER
		if (msg == "" || msg == string.Empty)
			return;

		if (!player || player.IsPlayerDisconnected() || !player.GetIdentity())
			return;

		Param1<string> m_MessageParam = new Param1<string>("");
		if (m_MessageParam && msg != "")
		{
			m_MessageParam.param1 = msg;
			GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, m_MessageParam, true, player.GetIdentity());
		}
#endif
	}

	//! Enable/disable player controls - WARNING: This function does not check if controls should be enabled given what the player is doing at the time when it's called, so use carefully and thoughtfully
	static void SetPlayerControl(bool isEnabled = true, bool hideHud = true)
	{
		#ifdef ZENMODPACK

		ZenFunctions.SetPlayerControl(isEnabled, hideHud);

		#else
		
		if (!GetGame())
			return;

		if (!GetGame().IsClient())
			return;

		if (!isEnabled)
		{
			GetGame().GetMission().AddActiveInputExcludes({"menu"});

			if (hideHud)
				GetGame().GetMission().GetHud().Show(false);
		}
		else
		{
			GetGame().GetMission().RemoveActiveInputExcludes({"menu"});

			if (hideHud)
				GetGame().GetMission().GetHud().Show(true);
		}

		#endif
	}

	static void EnablePlayerControl()
	{
		SetPlayerControl(true, false);
	}

	static void DisablePlayerControl()
	{
		SetPlayerControl(false, false);
	}
	
	/*
	This function essentially emulates durability increase. Because there is no efficient way to hook into an item's HP changes,
	(ie. there's no OnHealthChanged() or anything similair - only OnHealthLevelChanged) this function calculates what the odds
	are that the item would have X% increased durability if we were to track every single health impact on the item over its
	lifetime. 
	
	So essentially it takes the X% durability perk, and adjust its odds to suit the major health level changes directly 
	instead of refunding every single instance of damage dealt to the item (eg. like I was able to do with Clothing.c since that
	item is damaged by hits, and we can hook into its, but with tools we can't hook into Actions which is where the damage is 
	likely to be dealt (building/crafting/etc all simply use AddHealth() which we can't hook into).
	
	It's not perfect, but it's the next best thing to having a constantly running Timer or CallQueue checking for HP drops on 
	all items a player has in their inventory which seems to be to be overkill and a potential drag on server performance. 
	Note: this comes from ChatGPT, I'm not smart enough to come up with this on my own.
	
	TLDR: This function simulates durability increase on a macro health level change (WORN/DAMAGED/BADLY DAMAGED ETC) 
	      instead of refunding micro damages.
	*/
	static void HandleZenSkillsDurabilityPerk(EntityAI entity, bool mustBeInHands, int oldLevel, int newLevel, string zone, string skill, string perk)
	{
		if (newLevel == GameConstants.STATE_PRISTINE)
			return;
		
		if (!entity)
			return;
		
		PlayerBase player = PlayerBase.Cast(entity.GetHierarchyRootPlayer());
		if (!player)
			return;
		
		if (mustBeInHands && entity != player.GetItemInHands())
			return;

		ZenSkillsPlayerDB db = player.GetZenSkillsDB();
		if (!db)
			return;
		
		float perkBoostChance = db.GetPerkRewardPercent01(skill, perk);

		// Chance for the perk to trigger each time an item drops a health level.
		// Tuned so that, on average, this gives about +X% total durability.
		float diceRoll = 1.0 - 1.0 / (1.0 + perkBoostChance); // eg. 15% perk = ~0.1304
		
		#ifdef ZENSKILLSDEBUG
		string debugMsg = "HandleZenSkillsDurabilityPerk: " + entity.GetType();
		debugMsg = debugMsg + " skill=" + skill;
		debugMsg = debugMsg + " perk=" + perk;
		debugMsg = debugMsg + " perkBoostChance=" + perkBoostChance;
		debugMsg = debugMsg + " newLevel=" + newLevel;
		debugMsg = debugMsg + " oldLevel=" + oldLevel;
		debugMsg = debugMsg + " diceRoll=" + diceRoll;
		ZenSkillsPrint(debugMsg);
		#endif

        // downgrade only (higher number = worse)
        if (newLevel <= oldLevel)
		{
            return;
		}

        int levelsDropped = newLevel - oldLevel;
        int restores = 0;

        // independent per-level rolls; stop at first failure
        for (int i = 0; i < levelsDropped; i++)
        {
			float diceRollCheck = Math.RandomFloat01();
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[" + i + "] DiceRoll=" + diceRollCheck + " < " + diceRoll);
			#endif
			
            if (diceRollCheck < diceRoll)
			{
                restores++;
            } else
			{
                break;
			}
        }

        if (restores > 0)
        {
            int maxLevel = entity.GetNumberOfHealthLevels(zone) - 1;
			int targetLevel = Math.Clamp(newLevel - restores, GameConstants.STATE_PRISTINE, maxLevel);
			
			float top01 = entity.GetHealthLevelValue(targetLevel, zone); // top of band
			entity.SetHealth01(zone, "", top01);
			entity.SetSynchDirty();
			
			//entity.AddHealth(zone, "", entity.GetMaxHealth() * 0.01);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ZenSyncItemHealth, 10, false, entity);
		}
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("Item health level restores: " + restores + " levelsDropped=" + levelsDropped);
		#endif
	}
	
	static void ZenSetPristine(Object object)
	{
		if (!object)
			return;
	}
	
	// Typical weird DayZ behaviour - the health doesn't sync to client after setting health until damage is applied?
	// TODO: Maybe I'm retarded but this was the only way I could get the HP to sync on the tool after applying my
	// health bounce. Could have something to do with vanilla EEHealthLevelChanged syncing over-riding my update on same tick?
	// As usual, a simple idea is not nearly as simple to implement as anticipated -.-
	static void ZenSyncItemHealth(EntityAI entity)
	{
		if (!entity)
			return;
		
		entity.AddHealth("", "", -entity.GetMaxHealth() * 0.01); // addhealthlevel doesn't sync itself for some reason
		entity.SetSynchDirty();
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("HEALTH SYNC");
		#endif
	}
	
	// NOTE: Sometimes EEKilled says the entity killed itself for certain weapons... no idea why.
	// If it is more prevalent than I expect I may need to move this to EEHitBy() and detect !IsAlive()?
	static void HandleEntityKilledEXP(EntityAI deadEntity, Object killer)
	{
		if (!killer || !deadEntity)
			return;
		
		#ifdef ZENSKILLSDEBUG 
		if (killer)
		{
			ZenSkillsPrint("EEKilled - deadEntity=" + deadEntity.GetType() + " killer=" + killer.GetType() + " killerIsMan=" + killer.IsMan());
		}
		#endif
		
		if (deadEntity == killer)
			return;
		
		EntityAI eaiKiller = EntityAI.Cast(killer);
		if (!eaiKiller)
		{
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("NOT eaiKiller");
			#endif
			
			return;
		}
		
		PlayerBase player;
		
		if (eaiKiller.IsMan())
			player = PlayerBase.Cast(eaiKiller);
		
		if (!player)
			player = PlayerBase.Cast(eaiKiller.GetHierarchyRootPlayer());
		
		if (!player)
			return;
		
		#ifdef ZENSKILLSDEBUG 
		if (player)
		{
			ZenSkillsPrint("EEKilled " + deadEntity.GetType() + " hierarchyRootPlayerKillerFound=true");
		}
		#endif

		float multi = Math.Clamp(vector.Distance(player.GetPosition(), deadEntity.GetPosition()) / 100, 1, 10);
		
		string killType = "Killed_" + deadEntity.GetType();
		killType.ToLower();
		
		bool foundSpecificEntity = false;
		
		foreach (string skillKey, ZenSkillsEXPDefHolder skillExpDef : GetZenSkillsEXP().ExpDefs)
		{
			foreach (string key, ZenSkillsEXPDef expDef : skillExpDef.ExpDefs)
			{
				key.ToLower();
				
				if (key == killType)
				{
					foundSpecificEntity = true;
					break;
				}
			}
		}
		
		if (!foundSpecificEntity)
		{
			killType = GetKillType(deadEntity);
		}
		else 
		{
			ZombieBase zombie = ZombieBase.Cast(deadEntity);
			if (zombie && zombie.IsBeingBackstabbed())
			{
				killType.Replace("Killed_", "Stabbed_");
			}
		}
		
		if (ZEN_SKILLS_DEBUG_ON)
		{
			string msg = "EXP Key: " + killType + " Multi=" + multi + "x";
			ZenSkillsPrint(msg);
			ZenSkillFunctions.DebugMessage(msg);
		}
		
		if (killType == "Killed_Animal_Generic")
		{
			multi = deadEntity.GetMaxHealth() * multi;
		}

		GetZenSkillsPlugin().AddEXP_Action(player, killType, multi);
	}
	
	static string GetKillType(EntityAI deadEntity)
	{
		ZombieBase zombie = ZombieBase.Cast(deadEntity);
		if (zombie)
		{
			if (zombie.IsBeingBackstabbed())
				return "Stabbed_Zombie_Generic";
			
			return "Killed_Zombie_Generic";
		}
		
		AnimalBase animal = AnimalBase.Cast(deadEntity);
		if (animal)
		{
			return "Killed_Animal_Generic";
		}
		
		PlayerBase player = PlayerBase.Cast(deadEntity);
		if (player)
		{
			return "Killed_Player";
		}
		
		return "Killed_Entity_Generic";
	}
}



modded class ActionSkinningCB
{
	override void CreateActionComponent()
	{
		super.CreateActionComponent();
		
		float adjustedTime = UATimeSpent.SKIN;
		float speedBoost = 1 - m_ActionData.m_Player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_SKIN_SPEED);
		adjustedTime = adjustedTime * speedBoost;
		
		m_ActionData.m_ActionComponent = new CAContinuousTime(adjustedTime);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("ActionSkinningCB adjustedTime=" + adjustedTime + " speedBoost=" + speedBoost);
		#endif
	}
}

modded class ActionSkinning
{
	override ItemBase CreateOrgan(PlayerBase player, vector body_pos, string item_to_spawn, string cfg_skinning_organ_class, ItemBase tool)
	{
		ItemBase createdItem = super.CreateOrgan(player, body_pos, item_to_spawn, cfg_skinning_organ_class, tool);
		
		HandleZenSkills(createdItem, player, body_pos, item_to_spawn, cfg_skinning_organ_class, tool);

		return createdItem;
	}
	
	void HandleZenSkills(ItemBase createdItem, PlayerBase player, vector body_pos, string item_to_spawn, string cfg_skinning_organ_class, ItemBase tool)
	{
		Edible_Base food = Edible_Base.Cast(createdItem);
		Pelt_Base pelt = Pelt_Base.Cast(createdItem);
		WolfSteakMeat wolfMeat = WolfSteakMeat.Cast(food);
		BearSteakMeat bearMeat = BearSteakMeat.Cast(food);
		float purifyChance;
		float diceRoll;
		
		if (food && food.GetQuantityMax() > 1)
		{
			float qtyIncreasePct = player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_MEAT_QUANTITY);
			if (qtyIncreasePct > 0)
			{
				float qtyIncrease = food.GetQuantity() * qtyIncreasePct;
				food.AddQuantity(qtyIncrease);
				
				#ifdef ZENSKILLSDEBUG
				ZenSkillsPrint("Increase food qty=" + food.GetType() + " qtyIncrease=" + qtyIncrease + " %=" + qtyIncreasePct);
				#endif
			}
		}
		
		if (pelt)
		{
			float doubleChance = player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_PELT_QUANTITY);
			diceRoll = Math.RandomFloat01();
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("Increase pelt qty=" + pelt.GetType() + " doubleChance=" + doubleChance + " diceRoll=" + diceRoll);
			#endif
			
			if (diceRoll < doubleChance)
			{
				vector posHead;
				MiscGameplayFunctions.GetHeadBonePos(player, posHead);
				vector posRandom = MiscGameplayFunctions.GetRandomizedPositionVerified(posHead, body_pos, UAItemsSpreadRadius.NARROW, player);
				GetGame().CreateObjectEx(pelt.GetType(), posRandom, ECE_PLACE_ON_SURFACE);
			}
		}

		if (wolfMeat || bearMeat)
		{
			purifyChance = player.GetZenPerkRewardPercent01("hunting", ZenPerks.HUNTING_PREDATOR_MEAT);
			diceRoll = Math.RandomFloat01();
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("Purify predator meat=" + food.GetType() + " purifyChance=" + purifyChance + " diceRoll=" + diceRoll);
			#endif
			
			if (purifyChance > 0 && diceRoll < purifyChance)
			{
				food.SetZenSkillsNoSalmonella(true);
			}
		}
	}
}
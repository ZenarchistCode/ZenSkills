modded class PluginRepairing extends PluginBase
{
	/*
		1: Weapon Cleaning Kit
	    2: Sewing Kit
	    3: Leather Sewing Kit
	    4: Whetstone
	    5: Duct Tape
	    6: Tire Repair Kit
	    7: Electronic Repair Kit
	    8: Epoxy Putty
		9: DOESN'T EXIT
		10: Blowtorch
	*/
	
	override void CalculateHealth(PlayerBase player, ItemBase kit, Object item, float specialty_weight, string damage_zone = "", bool use_kit_qty = true)
	{
		float kitStartQty = 0;
		int repairKitType;
		
		if (kit)
		{
			kitStartQty = kit.GetQuantity();
			repairKitType = kit.ConfigGetInt("repairKitType");
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("PluginRepairing: kitStartQty=" + kitStartQty + " for kitType: " + repairKitType);
			#endif 
		}
		
		super.CalculateHealth(player, kit, item, specialty_weight, damage_zone, use_kit_qty);

		float perkRewardPct = 0;
		
		switch (repairKitType)
		{
			case 1: // Weapon Cleaning Kit
			case 6: // Tire Repair Kit
			case 7: // Electronic Repair Kit
			case 10: // Blowtorch
				perkRewardPct = player.GetZenPerkRewardPercent01("crafting", ZenPerks.CRAFTING_REPAIR_1_QTY);
				break;
			case 2: // Sewing Kit
			case 3: // Leather Sewing Kit
			case 5: // Duct Tape
				perkRewardPct = player.GetZenPerkRewardPercent01("crafting", ZenPerks.CRAFTING_REPAIR_2_QTY);
				break;
			case 4: // Whetstone
			case 8: // Epoxy Putty
				perkRewardPct = player.GetZenPerkRewardPercent01("crafting", ZenPerks.CRAFTING_REPAIR_3_QTY);
				break;
			default:
				break;
		}
		
		if (perkRewardPct <= 0)
			return;
	
		float kitDiff = kitStartQty - kit.GetQuantity();
		float kitRefundQty = kitDiff * perkRewardPct;
		kit.AddQuantity(kitRefundQty);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("PluginRepairing: perkRewardPct=" + perkRewardPct + " kitDiff=" + kitDiff + " kitRefundQty=" + kitRefundQty + " for kit: " + kit.GetType());
		#endif 
	}
	
	override bool Repair(PlayerBase player, ItemBase repair_kit, Object item, float specialty_weight, string damage_zone = "", bool use_kit_qty = true)
	{
		bool superRepair = super.Repair(player, repair_kit, item, specialty_weight, damage_zone, use_kit_qty);
		
		if (!superRepair)
			return false;
		
		if (item.GetHealthLevel() != GameConstants.STATE_WORN)
			return true;
		
		float chanceForPristine = player.GetZenPerkRewardPercent01("crafting", ZenPerks.CRAFTING_REPAIR_PRISTINE);
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Item health now: " + item.GetType() + " - " + item.GetHealth() + " level=" + item.GetHealthLevel() + " chanceForPristine=" + chanceForPristine);
		#endif
		
		if (Math.RandomFloat01() < chanceForPristine)
		{
			item.SetHealthLevel(GameConstants.STATE_PRISTINE);
			
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("Set pristine!");
			#endif 
		}
		
		return true;
	}
}
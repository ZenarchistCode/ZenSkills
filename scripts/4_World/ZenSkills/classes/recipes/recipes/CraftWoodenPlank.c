modded class CraftWoodenPlank
{
	override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
	{
		super.Do(ingredients, player, results, specialty_weight);
		
		float chance = player.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_EXTRA_PLANKS);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("Plank perk: chance=" + chance);
		#endif
		
		if (chance <= 0)
			return;
		
		if (Math.RandomFloat01() > chance)
			return;

		ItemBase secondItem = ItemBase.Cast(GetGame().CreateObjectEx(results.Get(0).GetType(), results.Get(0).GetPosition(), ECE_PLACE_ON_SURFACE));

		if (secondItem)
		{
			MiscGameplayFunctions.TransferItemProperties(results.Get(0), secondItem, false, true, true, true);
			secondItem.SetQuantity(results.Get(0).GetQuantity());
		}
	}
}
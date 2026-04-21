modded class CraftWoodenPlank
{
	override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
	{
		//! TEMPORARY FIX FOR VANILLA BUG: https://feedback.bistudio.com/T198350
		//super.Do(ingredients, player, results, specialty_weight);
		
		float chance = player.GetZenPerkRewardPercent01("crafting", ZenPerks.GATHERING_EXTRA_PLANKS);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("Survival perk: chance=" + chance);
		#endif
		
		if (chance > 0)
		{
			if (Math.RandomFloat01() <= chance)
			{
				ItemBase secondItem = ItemBase.Cast(g_Game.CreateObjectEx(results.Get(0).GetType(), results.Get(0).GetPosition(), ECE_PLACE_ON_SURFACE));

				if (secondItem)
				{
					MiscGameplayFunctions.TransferItemProperties(results.Get(0), secondItem, false, true, true, true);
					secondItem.SetQuantity(results.Get(0).GetQuantity());
				}
			}
		}
		
		super.Do(ingredients, player, results, specialty_weight);		
	}
}
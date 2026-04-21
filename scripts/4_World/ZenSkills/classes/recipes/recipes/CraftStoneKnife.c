modded class CraftStoneKnife
{	
	override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
	{
		//! TEMPORARY FIX FOR VANILLA BUG: https://feedback.bistudio.com/T198350
		//super.Do(ingredients, player, results, specialty_weight);
		
		float chance = player.GetZenPerkRewardPercent01("crafting", ZenPerks.CRAFTING_EXTRA_KNIFE);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("Survival perk: chance=" + chance);
		#endif
		
		if (chance > 0)
		{
			if (Math.RandomFloat01() <= chance)
			{
				ItemBase secondItem = ItemBase.Cast(g_Game.CreateObjectEx(results.Get(0).GetType(), results.Get(0).GetPosition(), ECE_PLACE_ON_SURFACE));
				MiscGameplayFunctions.TransferItemProperties(ingredients[0], secondItem, false, true, true, true);
			}
		}
		
		super.Do(ingredients, player, results, specialty_weight);
	}
}
modded class CraftBoneKnife
{	
	override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
	{
		super.Do(ingredients, player, results, specialty_weight);
		
		float chance = player.GetZenPerkRewardPercent01("crafting", ZenPerks.CRAFTING_EXTRA_KNIFE);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("Survival knife perk: chance=" + chance);
		#endif
		
		if (chance <= 0)
			return;
		
		if (Math.RandomFloat01() > chance)
			return;

		ItemBase secondItem = ItemBase.Cast(GetGame().CreateObjectEx(results.Get(0).GetType(), results.Get(0).GetPosition(), ECE_PLACE_ON_SURFACE));
		MiscGameplayFunctions.TransferItemProperties(ingredients[0], secondItem, false, true, true, true);
	}
}
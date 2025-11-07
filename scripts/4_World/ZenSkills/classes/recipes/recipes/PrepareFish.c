modded class PrepareFish
{
    override void AwardZenCraftingEXP(ItemBase ingredients[], PlayerBase player, array<ItemBase> spawned_objects)
	{
        GetZenSkillsPlugin().AddEXP_Action(player, "PrepareFish", GetLengthInSecs());
    }
}
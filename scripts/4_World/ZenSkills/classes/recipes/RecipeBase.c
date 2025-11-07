modded class RecipeBase 
{
	// Called even on recipes which don't actually spawn items (eg. repairing items) - after recipe is confirmed valid & sanity checked.
	override void SpawnItems(ItemBase ingredients[], PlayerBase player, array<ItemBase> spawned_objects)
	{
		super.SpawnItems(ingredients, player, spawned_objects);
		
		AwardZenCraftingEXP(ingredients, player, spawned_objects);
	}

	void AwardZenCraftingEXP(ItemBase ingredients[], PlayerBase player, array<ItemBase> spawned_objects)
	{
		if (!GetGame().IsDedicatedServer())
			return;
		
		string recipeName = ClassName();
		
		if (ZEN_SKILLS_DEBUG_ON)
		{
			string msg = "EXP Key: " + recipeName + " Multi=" + GetLengthInSecs() + "x";
			ZenSkillsPrint(msg);
			ZenSkillFunctions.DebugMessage(msg);
		}
		
		bool foundSpecificKey = false;
		recipeName.ToLower();
		
		foreach (string skillKey, ZenSkillsEXPDefHolder skillExpDef : GetZenSkillsEXP().ExpDefs)
		{
			foreach (string key, ZenSkillsEXPDef expDef : skillExpDef.ExpDefs)
			{
				key.ToLower();
				if (key == recipeName)
				{
					foundSpecificKey = true;
					break;
				}
			}
		}
		
		if (!foundSpecificKey)
		{
			recipeName = "GenericCrafting";
		}
		
		GetZenSkillsPlugin().AddEXP_Action(player, recipeName, GetLengthInSecs());
	}
}
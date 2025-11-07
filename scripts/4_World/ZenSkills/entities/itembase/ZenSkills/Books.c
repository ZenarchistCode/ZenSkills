class ZenSkills_BookBase extends ItemBase
{
    override void SetActions()
	{
		super.SetActions();

		AddAction(ActionReadZenSkillBook);
	}
}

class ZenSkills_Book_Survival extends ZenSkills_BookBase {};
class ZenSkills_Book_Crafting extends ZenSkills_BookBase {};
class ZenSkills_Book_Hunting extends ZenSkills_BookBase {};
class ZenSkills_Book_Gathering extends ZenSkills_BookBase {};

class ZenSkills_Book_Random extends ItemBase 
{
	override void EEInit()
	{
		super.EEInit();

		if (GetGame().IsDedicatedServer())
        {
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ZenShapeshifterMorph, 1000, false);
        }
	}

	private void ZenShapeshifterMorph()
	{
		string bookType = "ZenSkills_Book_" + GetZenSkillsConfig().SkillDefs.GetKeyArray().GetRandomElement();
		GetInventory().ReplaceItemWithNew(InventoryMode.SERVER, new ReplaceZenSkillBookWithNewLambda(this, bookType));
	}
}

class ReplaceZenSkillBookWithNewLambda : ReplaceItemWithNewLambdaBase
{
	void ReplaceZenSkillBookWithNewLambda(EntityAI old_item, string new_item_type)
	{
		m_OldItem = old_item;
		m_NewItemType = new_item_type;
	}

	override void OnSuccess(EntityAI new_item)
	{
		if (new_item != NULL)
		{
			MiscGameplayFunctions.TransferItemProperties(m_OldItem, new_item);
			new_item.SetLifetimeMax(14400);
			new_item.SetLifetime(14400);
		}
	}
}
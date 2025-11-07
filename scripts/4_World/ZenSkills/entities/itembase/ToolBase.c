modded class ToolBase 
{
	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		
		if (GetGame().IsDedicatedServer() && IsInitialized())
		{
			ZenSkillFunctions.HandleZenSkillsDurabilityPerk(this, true, oldLevel, newLevel, zone, "crafting", ZenPerks.CRAFTING_TOOL_DURABILITY);
		}
    }
}

// These tools don't extend ToolBase. Of course they don't, why would they? This is DayZ.
modded class Pickaxe 
{
	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		
		if (GetGame().IsDedicatedServer() && IsInitialized())
		{
			ZenSkillFunctions.HandleZenSkillsDurabilityPerk(this, true, oldLevel, newLevel, zone, "crafting", ZenPerks.CRAFTING_TOOL_DURABILITY);
		}
    }
}

modded class SledgeHammer 
{
	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		
		if (GetGame().IsDedicatedServer() && IsInitialized())
		{
			ZenSkillFunctions.HandleZenSkillsDurabilityPerk(this, true, oldLevel, newLevel, zone, "crafting", ZenPerks.CRAFTING_TOOL_DURABILITY);
		}
    }
}

modded class Iceaxe 
{
	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		
		if (GetGame().IsDedicatedServer() && IsInitialized())
		{
			ZenSkillFunctions.HandleZenSkillsDurabilityPerk(this, true, oldLevel, newLevel, zone, "crafting", ZenPerks.CRAFTING_TOOL_DURABILITY);
		}
    }
}

modded class CanOpener 
{
	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		
		if (GetGame().IsDedicatedServer() && IsInitialized())
		{
			ZenSkillFunctions.HandleZenSkillsDurabilityPerk(this, true, oldLevel, newLevel, zone, "crafting", ZenPerks.CRAFTING_TOOL_DURABILITY);
		}
    }
}
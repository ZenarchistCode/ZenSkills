// A little hacky, but cbf creating a custom class config for a simple edible item
// which will essentially do all the same things a mushroom can in terms of sustenance/cooking.
// Only potential problem is if some mod adds features to mushrooms, these berries will inherit its behaviour.
class ZenSkillsBerryBase extends MushroomBase
{
}

class ZenSkills_SambucusBerry extends ZenSkillsBerryBase {};
class ZenSkills_CaninaBerry extends ZenSkillsBerryBase {};

// This class handles spawning a new fruit if the perk is enabled and the dice roll succeeds
class ZenSkillsFruitManager 
{
	static void HandleFruitSpawnChance(EntityAI fruitShroomEAI, EntityAI potentialPlayer)
	{
		ItemBase fruitShroom = ItemBase.Cast(fruitShroomEAI);

		if (!potentialPlayer || !fruitShroom || fruitShroom.IsRuined())
			return;
		
		PlayerBase pb = PlayerBase.Cast(potentialPlayer);
		if (!pb)
		{
			pb = PlayerBase.Cast(potentialPlayer.GetHierarchyRootPlayer());
		}
		
		if (!pb)
			return;
		
		float extraSpawnChance = pb.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_EXTRA_FRUIT);
		if (extraSpawnChance <= 0)
			return;
		
		float diceRoll = Math.RandomFloat01();
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint(fruitShroom.GetType() + " picked up by player - perkChance=" + extraSpawnChance + " diceRoll=" + diceRoll);
		#endif
		
		if (diceRoll < extraSpawnChance)
		{
			vector pos = ZenFunctions.GetRandomPointInCircle(fruitShroom.GetPosition(), 1, 0.5);
			Edible_Base extraItem = Edible_Base.Cast(g_Game.CreateObjectEx(fruitShroom.GetType(), pos, ECE_SETUP));
			if (!extraItem)
			{
				Error("Failed to spawn " + fruitShroom.GetType());
				return;
			}
			
			extraItem.TriggerZenParticleDust();
		}
	}
}

modded class MushroomBase
{
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner, new_owner);
		
		if (!IsZenVirgin() || !g_Game.IsDedicatedServer() || !new_owner)
			return;
		
		ZenSkillsFruitManager.HandleFruitSpawnChance(this, new_owner);
	}
}

modded class Pear
{
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner, new_owner);
		
		if (!IsZenVirgin() || !g_Game.IsDedicatedServer() || !new_owner)
			return;
		
		ZenSkillsFruitManager.HandleFruitSpawnChance(this, new_owner);
	}
}

modded class Apple
{
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner, new_owner);
		
		if (!IsZenVirgin() || !g_Game.IsDedicatedServer() || !new_owner)
			return;
		
		ZenSkillsFruitManager.HandleFruitSpawnChance(this, new_owner);
	}
}

modded class Plum
{
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner, new_owner);
		
		if (!IsZenVirgin() || !g_Game.IsDedicatedServer() || !new_owner)
			return;
		
		ZenSkillsFruitManager.HandleFruitSpawnChance(this, new_owner);
	}
}
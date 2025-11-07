// A little hacky, but cbf creating a custom class config for a simple edible item
// which will essentially do all the same things a mushroom can in terms of sustenance/cooking.
// Only potential problem is if some mod adds features to mushrooms, these berries will inherit its behaviour.
class ZenSkillsBerryBase extends MushroomBase
{
}

class ZenSkills_SambucusBerry extends ZenSkillsBerryBase {};
class ZenSkills_CaninaBerry extends ZenSkillsBerryBase {};

class ZenSkillsFruitManager 
{
	static void HandleFruitSpawnChance(EntityAI fruitShroom, EntityAI potentialPlayer)
	{
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
			EntityAI extraItem = EntityAI.Cast(GetGame().CreateObjectEx(fruitShroom.GetType(), fruitShroom.GetPosition(), ECE_PLACE_ON_SURFACE));
		}
	}
}

modded class MushroomBase
{
	bool m_ZenSkillsIsVirgin = false;
	
	override void EEOnCECreate()
	{
		super.EEOnCECreate();

		if (GetParent() != null)
			return;
		
		m_ZenSkillsIsVirgin = true;
	}
	
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner, new_owner);
		
		if (!m_ZenSkillsIsVirgin || !GetGame().IsDedicatedServer() || !new_owner)
			return;
		
		ZenSkillsFruitManager.HandleFruitSpawnChance(this, new_owner);
	}
	
	//! PERSISTENCE NOTE:
	// I tried to stay away from onstoreload/save in this mod to make it easy to add/remove without wiping server,
	// but couldn't think of a way around this one. I made an exception because hardly anyone would keep fruit in storage
	// and even if it's corrupted during OnStoreLoad, it shouldn't break anything important.
	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		
		ctx.Write(m_ZenSkillsIsVirgin);
	}
	
	override bool OnStoreLoad(ParamsWriteContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;
		
		if (!ctx.Read(m_ZenSkillsIsVirgin))
			return false;
			
		return true;
	}
}

modded class Pear
{
	bool m_ZenSkillsIsVirgin = false;
	
	override void EEOnCECreate()
	{
		super.EEOnCECreate();

		if (GetParent() != null)
			return;
		
		m_ZenSkillsIsVirgin = true;
	}
	
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner, new_owner);
		
		if (!m_ZenSkillsIsVirgin || !GetGame().IsDedicatedServer() || !new_owner)
			return;
		
		ZenSkillsFruitManager.HandleFruitSpawnChance(this, new_owner);
	}
	
	//! PERSISTENCE NOTE:
	// I tried to stay away from onstoreload/save in this mod to make it easy to add/remove without wiping server,
	// but couldn't think of a way around this one. I made an exception because hardly anyone would keep fruit in storage
	// and even if it's corrupted during OnStoreLoad, it shouldn't break anything important.
	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		
		ctx.Write(m_ZenSkillsIsVirgin);
	}
	
	override bool OnStoreLoad(ParamsWriteContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;
		
		if (!ctx.Read(m_ZenSkillsIsVirgin))
			return false;
			
		return true;
	}
}

modded class Apple
{
	bool m_ZenSkillsIsVirgin = false;
	
	override void EEOnCECreate()
	{
		super.EEOnCECreate();

		if (GetParent() != null)
			return;
		
		m_ZenSkillsIsVirgin = true;
	}
	
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner, new_owner);
		
		if (!m_ZenSkillsIsVirgin || !GetGame().IsDedicatedServer() || !new_owner)
			return;
		
		ZenSkillsFruitManager.HandleFruitSpawnChance(this, new_owner);
	}
	
	//! PERSISTENCE NOTE:
	// I tried to stay away from onstoreload/save in this mod to make it easy to add/remove without wiping server,
	// but couldn't think of a way around this one. I made an exception because hardly anyone would keep fruit in storage
	// and even if it's corrupted during OnStoreLoad, it shouldn't break anything important.
	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		
		ctx.Write(m_ZenSkillsIsVirgin);
	}
	
	override bool OnStoreLoad(ParamsWriteContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;
		
		if (!ctx.Read(m_ZenSkillsIsVirgin))
			return false;
			
		return true;
	}
}

modded class Plum
{
	bool m_ZenSkillsIsVirgin = false;
	
	override void EEOnCECreate()
	{
		super.EEOnCECreate();

		if (GetParent() != null)
			return;
		
		m_ZenSkillsIsVirgin = true;
	}
	
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{
		super.OnItemLocationChanged(old_owner, new_owner);
		
		if (!m_ZenSkillsIsVirgin || !GetGame().IsDedicatedServer() || !new_owner)
			return;
		
		ZenSkillsFruitManager.HandleFruitSpawnChance(this, new_owner);
	}
	
	//! PERSISTENCE NOTE:
	// I tried to stay away from onstoreload/save in this mod to make it easy to add/remove without wiping server,
	// but couldn't think of a way around this one. I made an exception because hardly anyone would keep fruit in storage
	// and even if it's corrupted during OnStoreLoad, it shouldn't break anything important.
	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		
		ctx.Write(m_ZenSkillsIsVirgin);
	}
	
	override bool OnStoreLoad(ParamsWriteContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;
		
		if (!ctx.Read(m_ZenSkillsIsVirgin))
			return false;
			
		return true;
	}
}
modded class CAContinuousMineWood extends CAContinuousBase
{
    protected string GetZenSkillKey(string material = "")
    {
        return "gathering";
    }
	
	array<string> GetZenBerries()
	{
		array<string> berries = new array<string>;
		berries.Insert("ZenSkills_SambucusBerry");
		berries.Insert("ZenSkills_CaninaBerry");
		return berries;
	}

    // Map output material -> relevant perk key
    protected string ZenGetBonusPerkKeyForMaterial(string material)
    {
        if (material == "" ) 
			return "";

        // Stones
        if (material == "Stone")
            return ZenPerks.GATHERING_EXTRA_ROCKS;

        // Logs
        if (material == "WoodenLog")
            return ZenPerks.GATHERING_EXTRA_LOGS;

        // Sticks -> berries
        if (material.Contains("WoodenStick"))
            return ZenPerks.GATHERING_BERRIES;

        return "";
    }

    // Decide what extra item to drop for a given material.
    // Return "" to skip. Default: same resource except sticks -> random berry.
    protected string ZenGetBonusDropTypeForMaterial(ActionData action_data, string material)
    {
        if (material == "") 
			return "";

        if (material.Contains("WoodenStick"))
        {
            // random berry
            return GetZenBerries().GetRandomElement();
        }

        // For logs/stones, default to "same as mined"
        return material;
    }

    // Quantity of the bonus when spawning a separate item (default 1)
    protected int ZenGetBonusDropQuantity(string dropType, string sourceMaterial)
    {
        return 1;
    }

    // At most one bonus spawn per tick? (default true; override if you want many)
    protected bool ZenSpawnAtMostOncePerTick()
    {
        return true;
    }

    override void Setup(ActionData action_data)
    {
        super.Setup(action_data);
		
        #ifdef ZENSKILLSDEBUG
        ZenSkillsPrint("[" + ClassName() + ".Setup] drops=" + m_AmountOfDrops + " tBetween=" + m_TimeBetweenMaterialDrops);
        #endif
    }

    override void CreatePrimaryItems(ActionData action_data)
	{
		super.CreatePrimaryItems(action_data);
		
		ZenCreateExtraItems(action_data);	
	}
	
	void ZenCreateExtraItems(ActionData action_data)
	{
		if (!GetGame().IsDedicatedServer())
			return;

		if (!action_data || !action_data.m_Player)
			return;

		if (!m_MaterialAndQuantityMap || m_MaterialAndQuantityMap.Count() == 0)
			return;
	
		int spawnsRemaining;
		if (ZenSpawnAtMostOncePerTick())
		{
			spawnsRemaining = 1;
		}
		else
		{
			spawnsRemaining = m_MaterialAndQuantityMap.Count();
		}
	
		for (int i = 0; i < m_MaterialAndQuantityMap.Count() && spawnsRemaining > 0; i++)
		{
			string mat = m_MaterialAndQuantityMap.GetKey(i);
			string perkKey = ZenGetBonusPerkKeyForMaterial(mat);
			if (perkKey == "")
				continue;
	
			float chance01 = action_data.m_Player.GetZenPerkRewardPercent01(GetZenSkillKey(), perkKey);
			if (chance01 <= 0.0)
				continue;
	
			float roll = Math.RandomFloat01();
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[" + ClassName() + "] mat=" + mat + " perk=" + perkKey + " chance=" + chance01 + " roll=" + roll);
			#endif
	
			if (roll >= chance01)
				continue;
	
			string dropType = ZenGetBonusDropTypeForMaterial(action_data, mat);
			if (dropType == "")
				continue;
	
			if (dropType == mat)
			{
				ItemBase mined = m_MinedItem[i];
				if (mined)
				{
					if (mined.HasQuantity())
					{
						if (mined.IsFullQuantity())
						{
							ItemBase extraA = ItemBase.Cast(GetGame().CreateObjectEx(dropType, action_data.m_Player.GetPosition(), ECE_PLACE_ON_SURFACE));
							if (extraA && extraA.HasQuantity())
								extraA.SetQuantity(1, false);
						}
						else
						{
							mined.AddQuantity(1, false);
						}
					}
					else
					{
						ItemBase extraB = ItemBase.Cast(GetGame().CreateObjectEx(dropType, action_data.m_Player.GetPosition(), ECE_PLACE_ON_SURFACE));
					}
				}
				else
				{
					ItemBase extraC = ItemBase.Cast(GetGame().CreateObjectEx(dropType, action_data.m_Player.GetPosition(), ECE_PLACE_ON_SURFACE));
					if (extraC && extraC.HasQuantity())
						extraC.SetQuantity(1, false);
				}
			}
			else
			{
				int qty = ZenGetBonusDropQuantity(dropType, mat);
				if (qty < 1)
					qty = 1;
	
				vector pos = action_data.m_Player.GetPosition();
	
				EntityAI spawned = EntityAI.Cast(GetGame().CreateObjectEx(dropType, pos, ECE_PLACE_ON_SURFACE));
				if (spawned)
				{
					ItemBase ib = ItemBase.Cast(spawned);
					if (ib && ib.HasQuantity())
						ib.SetQuantity(qty, false);
	
					spawned.SetLifetimeMax(900);
					spawned.SetLifetime(900);
	
					#ifdef ZENSKILLSDEBUG
					ZenSkillsPrint("[" + ClassName() + "] BONUS spawn type=" + dropType + " qty=" + qty + " from mat=" + mat);
					#endif
				}
			}
	
			spawnsRemaining = spawnsRemaining - 1;
		}
	}
}

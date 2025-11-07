class ZenMatSnap
{
    string slotName;
    string itemType;
    float preQty;
    float preQtyMax;
    bool hadItem;
    bool isLockable;
    float qtyConfig; // -1 => delete-on-use
}

modded class Construction
{
    override void BuildPartServer(notnull Man player, string part_name, int action_id)
    {
        // Snapshot BEFORE vanilla consumes
        array<ref ZenMatSnap> snaps;
        ZenCaptureMaterialSnapshot(part_name, snaps);

        super.BuildPartServer(player, part_name, action_id);

        PlayerBase pb = PlayerBase.Cast(player);
        if (!pb) 
			return;

        ZenSkillsPlayerDB db = pb.GetZenSkillsDB();
        if (!db) 
			return;

        float triggerChance = Math.Clamp(db.GetPerkRewardPercent01("crafting", ZenPerks.CRAFTING_BUILD_REFUND), 0.0, 1.0);
        if (triggerChance <= 0) 
			return;

        float roll = Math.RandomFloat01();
        if (roll >= triggerChance) 
			return;

        float refundScale = ZenSkillConstants.ZEN_BUILDING_PERK_REFUND_AMT; // 0.5 = 50%
        ZenRefundFromSnapshot(snaps, refundScale);
    }
	
	static int ZenStochasticRound(float x)
    {
        int baseVal = Math.Floor(x);
        float frac = x - baseVal;
        float roll = Math.RandomFloat01();
        if (roll < frac) 
			return baseVal + 1;
		
        return baseVal;
    }

    void ZenCaptureMaterialSnapshot(string partName, out array<ref ZenMatSnap> outSnaps)
    {
        outSnaps = new array<ref ZenMatSnap>();

        string mainPart = GetConstructionPart(partName).GetMainPartName();
        string cfgPath = "cfgVehicles" + " " + GetParent().GetType() + " " + "Construction" + " " + mainPart + " " + partName + " " + "Materials";
		
        if (!GetGame().ConfigIsExisting(cfgPath)) 
			return;

        int childCount = GetGame().ConfigGetChildrenCount(cfgPath);
        for (int i = 0; i < childCount; i++)
        {
            string childName;
            GetGame().ConfigGetChildName(cfgPath, i, childName);

            string path;
            string slotName;

            path = cfgPath + " " + childName + " " + "slot_name";
            GetGame().ConfigGetText(path, slotName);

            path = cfgPath + " " + childName + " " + "quantity";
            float qtyConfig = GetGame().ConfigGetFloat(path);

            path = cfgPath + " " + childName + " " + "lockable";
            bool lockable = GetGame().ConfigGetInt(path);

            ItemBase att = ItemBase.Cast(GetParent().FindAttachmentBySlotName(slotName));

            ZenMatSnap s = new ZenMatSnap();
            s.slotName = slotName;
            s.isLockable = lockable;
            s.qtyConfig = qtyConfig;

            if (att)
            {
                s.itemType = att.GetType();
                s.preQty = att.GetQuantity();
                s.preQtyMax = att.GetQuantityMax();
                s.hadItem = true;
            }
            else
            {
                s.itemType = ""; // unknown until we see post state (slot empty pre-build should not happen normally)
                s.preQty = 0;
                s.preQtyMax = 0;
                s.hadItem = false;
            }

            outSnaps.Insert(s);
        }
    }

    void ZenRefundFromSnapshot(array<ref ZenMatSnap> snaps, float refundScale)
    {
        if (!snaps) 
			return;

        for (int i = 0; i < snaps.Count(); i++)
        {
            ZenMatSnap s = snaps[i];

            if (s.isLockable) 
				continue;
			
            if (s.qtyConfig <= -1) 
				continue; // delete-on-use: never refund

            ItemBase attNow = ItemBase.Cast(GetParent().FindAttachmentBySlotName(s.slotName));

            float consumed = 0;
            string itemTypeUsed = s.itemType;

            if (attNow)
            {
                // If stack still exists, infer used from delta. If it is a different type, assume old stack fully consumed.
                if (s.hadItem && attNow.GetType() == s.itemType)
                {
                    float nowQty = attNow.GetQuantity();
                    float delta = s.preQty - nowQty;
					
                    if (delta > 0) 
						consumed = delta;
                }
                else 
				if (s.hadItem)
                {
                    consumed = s.preQty;
                }
                else
                {
                    // No pre item recorded; nothing to refund safely.
                    continue;
                }
            }
            else
            {
                // Stack was fully consumed and removed.
                if (!s.hadItem) 
					continue;
				
                consumed = s.preQty;
            }

            if (consumed <= 0) 
				continue;

            int refundUnits = ZenStochasticRound(consumed * refundScale);
            if (refundUnits <= 0) 
				continue;

            if (attNow)
            {
                float room = attNow.GetQuantityMax() - attNow.GetQuantity();
                int addUnits = Math.Floor(Math.Min(room, refundUnits));
				
                if (addUnits > 0) 
					attNow.AddQuantity(addUnits);
            }
            else
            {
                // Respawn a new stack in the same slot; fallback to ground if attach fails.
                if (itemTypeUsed.Length() == 0) 
					continue;

                int slotId = InventorySlots.GetSlotIdFromString(s.slotName);
                EntityAI spawned = EntityAI.Cast(GetParent().GetInventory().CreateAttachmentEx(itemTypeUsed, slotId));

                if (!spawned)
                {
                    vector pos = GetParent().GetPosition();
                    spawned = EntityAI.Cast(GetGame().CreateObject(itemTypeUsed, pos));
                }

                ItemBase newStack = ItemBase.Cast(spawned);
                if (newStack)
                {
                    float setQty = Math.Min(refundUnits, newStack.GetQuantityMax());
                    if (setQty < 1) 
						setQty = 1; // safeguard for items that require integer at least 1
					
                    newStack.SetQuantity(setQty);
                }
            }

            #ifdef ZENSKILLSDEBUG
            ZenSkillsPrint("Refunded " + refundUnits + " units into slot " + s.slotName);
            #endif
        }
    }
}
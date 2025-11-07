modded class ActionSawPlanks
{
	protected int ZenGetPlanksBonusAmount()
	{
		return YIELD;
	}
	
	override void OnFinishProgressServer(ActionData action_data)
	{
		// Run vanilla behavior first
		super.OnFinishProgressServer(action_data);

		SawPlanksActionData data = SawPlanksActionData.Cast(action_data);
		if (!data || !data.m_Player)
			return;

		float chance = data.m_Player.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_EXTRA_PLANKS);

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[SawPlanks] double chance=" + chance);
		#endif

		if (chance <= 0.0)
			return;

		float roll = Math.RandomFloat01();
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[SawPlanks] roll=" + roll);
		#endif
		
		if (roll >= chance)
			return;

		int bonus = ZenGetPlanksBonusAmount();
		if (bonus < 1)
			return;

		// Give extra planks (stack if possible, otherwise spawn additional pile)
		ZenGiveExtraPlanks(data, bonus);
	}

	protected void ZenGiveExtraPlanks(SawPlanksActionData data, int amount)
	{
		// Use same stacking semantics as vanilla
		if (!data.m_LastPlanksPile)
		{
			SpawnNewPlankPile(data, amount);
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[SawPlanks] bonus spawned new pile qty=" + amount);
			#endif
			return;
		}

		ItemBase pile = data.m_LastPlanksPile;
		int maxq = pile.GetQuantityMax();
		int curq = pile.GetQuantity();
		int after = curq + amount;

		if (pile.HasQuantity())
		{
			if (after >= maxq)
			{
				int rem = after - maxq;
				pile.SetQuantity(maxq);
				if (rem > 0)
				{
					SpawnNewPlankPile(data, rem);
				}
				#ifdef ZENSKILLSDEBUG
				ZenSkillsPrint("[SawPlanks] bonus filled stack to " + maxq + ", remnant=" + rem);
				#endif
			}
			else
			{
				pile.AddQuantity(amount, false);
				#ifdef ZENSKILLSDEBUG
				ZenSkillsPrint("[SawPlanks] bonus added +" + amount + " to existing pile");
				#endif
			}
		}
		else
		{
			// Non-quantity safety; planks are quantity-based but keep fallback
			ItemBase extra = ItemBase.Cast(data.m_Player.SpawnEntityOnGroundRaycastDispersed("WoodenPlank", 0.3, UAItemsSpreadRadius.VERY_NARROW));
			if (extra)
			{
				extra.SetQuantity(amount);
				data.m_LastPlanksPile = extra;
			}
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[SawPlanks] bonus created extra non-quantity item");
			#endif
		}
	}
}
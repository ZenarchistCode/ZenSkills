modded class OpenAndSwitchLambda
{
	override void OnSuccess(EntityAI new_item)
	{
		ItemBase ib = ItemBase.Cast(new_item);
		
		if (!ib || !m_Player)
		{
			super.OnSuccess(new_item);
			return;
		}
		
		float quantity_old = ib.GetQuantity();
		float spill_amount = quantity_old * m_SpillModifier;
		
		super.OnSuccess(new_item);
		
		float perkQty = m_Player.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_CANNED_FOOD);
		if (perkQty <= 0)
			return;
		
		float qtyDiff = quantity_old - ib.GetQuantity();
		float refund = qtyDiff * perkQty;
		ib.AddQuantity(refund);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("OpenAndSwitchLambda perkQty=" + perkQty + " qtyDiff=" + quantity_old + " refund=" + qtyDiff);
		#endif
	}
}
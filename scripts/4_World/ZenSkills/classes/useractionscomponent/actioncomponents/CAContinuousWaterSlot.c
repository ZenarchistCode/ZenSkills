modded class ActionWaterGardenSlotCB
{
	override void CreateActionComponent()
	{
		super.CreateActionComponent();

		float adjustedQty = QUANTITY_USED_PER_SEC;
		float qtyReduction = 1 - m_ActionData.m_Player.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_PLANT_FERTILIZER);
		adjustedQty = adjustedQty * qtyReduction;
		
		m_ActionData.m_ActionComponent = new CAContinuousWaterSlot(adjustedQty);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("ActionWaterGardenSlotCB adjustedQty=" + adjustedQty + " qtyReduction=" + qtyReduction);
		#endif
	}
}

modded class ActionFertilizeSlotCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		super.CreateActionComponent();

		float adjustedQty = QUANTITY_USED_PER_SEC;
		float qtyReduction = 1 - m_ActionData.m_Player.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_PLANT_FERTILIZER);
		adjustedQty = adjustedQty * qtyReduction;
		
		m_ActionData.m_ActionComponent = new CAContinuousFertilizeGardenSlot(adjustedQty);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("ActionFertilizeSlotCB adjustedQty=" + adjustedQty + " qtyReduction=" + qtyReduction);
		#endif
		
		
	}
}
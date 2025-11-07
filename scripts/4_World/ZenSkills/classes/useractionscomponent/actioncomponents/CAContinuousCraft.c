modded class CAContinuousCraft
{		
	override void Setup(ActionData action_data)
	{
		super.Setup(action_data);
		
		float perk = action_data.m_Player.GetZenPerkRewardPercent01("crafting", ZenPerks.CRAFTING_SPEED_BOOST);
		
		if (perk <= 0)
			return;
		
		float speedBoost = 1 - perk;
		m_AdjustedTimeToComplete = m_AdjustedTimeToComplete * speedBoost;
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("CAContinuousCraft m_AdjustedTimeToComplete=" + m_AdjustedTimeToComplete + " speedBoost=" + speedBoost + "%");
		#endif
	}
}
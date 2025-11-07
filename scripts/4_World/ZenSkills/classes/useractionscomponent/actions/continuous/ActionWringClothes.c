modded class ActionWringClothesCB
{
	override void CreateActionComponent()
	{
		super.CreateActionComponent();

        float pct = 1 - m_ActionData.m_Player.GetZenPerkRewardPercent01("survival", ZenPerks.SURVIVAL_WRING_CLOTHES);
        if (pct <= 0) 
			return;
		
		int timeSpent = UATimeSpent.WRING * pct;
		m_ActionData.m_ActionComponent = new CAContinuousRepeat(timeSpent);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("Wring clothes perk time: " + timeSpent + " (" + pct + "%)");
		#endif
	}
}
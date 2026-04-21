class ActionInjectZenSkillsExpBoostSelf: ActionInjectSelf
{
	override void ApplyModifiers(ActionData action_data)
	{
        if (g_Game.IsDedicatedServer())
        {
            action_data.m_MainItem.OnApply(action_data.m_Player);
        }
	}
}
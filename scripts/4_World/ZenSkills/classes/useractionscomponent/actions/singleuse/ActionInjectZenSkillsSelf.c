class ActionInjectZenSkillsSelf: ActionInjectSelf
{
	override void ApplyModifiers(ActionData action_data)
	{
        if (GetGame().IsDedicatedServer())
        {
            action_data.m_MainItem.OnApply(action_data.m_Player);
        }
	}
}
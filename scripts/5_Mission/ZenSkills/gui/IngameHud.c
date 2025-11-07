modded class IngameHud 
{
	override void Init(Widget hud_panel_widget)
	{
		super.Init(hud_panel_widget);
		
		if (!m_ZenSkillsHUD)
		{
			m_ZenSkillsHUD = new ZenSkillsHUD();
		}
	}
}
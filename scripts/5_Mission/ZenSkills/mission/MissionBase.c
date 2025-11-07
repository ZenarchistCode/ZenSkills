modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
    {
        UIScriptedMenu menu = super.CreateScriptedMenu(id);

        if (!menu)
        {
            switch (id)
            {
                case ZenSkillConstants.SKILL_GUI:
                {
                    menu = new ZenSkillsGUI();
					menu.SetID(id);
                    break;
                }
				
				case ZenSkillConstants.SKILL_HIGHSCORES:
                {
                    menu = new ZenSkillsHighscores();
					menu.SetID(id);
                    break;
                }
				
				case ZenSkillConstants.SKILL_TUTORIAL:
				{
					menu = new ZenSkillsTutorial();
					menu.SetID(id);
                    break;
				}
            }
        }

        return menu;
    }
}
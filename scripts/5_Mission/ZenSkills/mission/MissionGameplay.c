// Client-side only code
modded class MissionGameplay
{
	override void OnInit()
    {
        super.OnInit();
		
		GetZenSkillsClientConfig();
    }
	
	override void OnMissionFinish()
	{
		super.OnMissionFinish();
		
		GetZenSkillsClientConfig().Save();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice); 

		if (!g_Game)
			return;

		UpdateZenSkillsInput();
		
		if (ZenSkillsPlayerDB.RECEIVED_DATA)
		{
			UpdateZenSkillsHUD();
		}
	}
	
	void UpdateZenSkillsHUD()
	{
		if (!m_Hud) 
		{
			m_Hud.GetZenSkillsHUD().HideAll();
			return;
		}

		if (!g_Game.GetPlayer() || !g_Game.GetPlayer().IsAlive() || g_Game.GetPlayer().IsUnconscious())
		{
			m_Hud.GetZenSkillsHUD().HideAll();
			return;
		}

		if (m_Hud.GetHudVisibility().IsContextFlagActive(IngameHudVisibility.HUD_HIDE_FLAGS) || m_Hud.GetHudVisibility().IsContextFlagActive(EHudContextFlags.INVENTORY_OPEN))
		{
			if (m_Hud.GetZenSkillsHUD())
			{
				m_Hud.GetZenSkillsHUD().HideAll();
			}

			return;
		}

		if (g_Game.GetUIManager().GetMenu() != NULL) 
		{
			if (m_Hud.GetZenSkillsHUD())
			{
				m_Hud.GetZenSkillsHUD().HideAll();
			}
		}
	}

	/*override void OnKeyPress(int key)
	{
		super.OnKeyPress(key);

		Print("Key pressed ID: " + key);
		
		if (key == 25) // p key
		{
			ZenSkillsHighscores hs = ZenSkillsHighscores.Cast(g_Game.GetUIManager().EnterScriptedMenu(ZenSkillConstants.SKILL_HIGHSCORES, NULL));
		}
	}*/

	void UpdateZenSkillsInput()
	{
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());

        if (!player)
			return;
		
		if (!player.IsAlive() || player.IsUnconscious())
			return;

		if (ZenSkills_CheckInput(ZenSkillConstants.KEY_INPUT_OPEN_SKILLS_GUI))
        {
            if (g_Game.GetUIManager() != NULL)
            {
                ZenSkillsGUI gui = ZenSkillsGUI.Cast(g_Game.GetUIManager().EnterScriptedMenu(ZenSkillConstants.SKILL_GUI, NULL));
            }
        }
	}

    bool ZenSkills_CheckInput(string inputName)
    {
        if (GetUApi())
		{
            UAInput uai = GetUApi().GetInputByName(inputName);

            if (uai && uai.LocalPress())
            {
                return true;
            }
        }

        return false;
    }
}
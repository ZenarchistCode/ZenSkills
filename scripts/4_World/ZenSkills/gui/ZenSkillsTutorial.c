class ZenSkillsTutorial extends UIScriptedMenu
{
	static const string LAYOUT_FILE = "ZenSkills/data/gui/layouts/zen_skills_tutorial.layout";
	
	AbstractWave m_Sound;

	// Main widgets
	ref TextWidget m_DescriptionTitle;
	ref MultilineTextWidget m_DescriptionLabel;
	ref ButtonWidget m_OkButton;

	override Widget Init()
	{
	    layoutRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT_FILE);

		if (!ZenSkillsPlayerDB.RECEIVED_DATA)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Close, 10);
			return layoutRoot;
		}
		
		PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayer());
		if (!pb || !pb.GetIdentity())
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Close, 10);
			return layoutRoot;
		}
		
		string titleText = Widget.TranslateString("#STR_ZenSkills_Welcome");
		titleText = string.Format(titleText, pb.GetIdentity().GetName());
		
		int expLossPercent = (int)Math.Round(GetZenSkillsConfig().SharedConfig.PercentOfExpLostOnDeath * 100);
		
		string desc = Widget.TranslateString("#STR_ZenSkills_GUI_ModDescription");
		desc = string.Format(desc, "#STR_ZenSkills_GUI_ShowExp", expLossPercent.ToString() + "%");
	
		m_DescriptionTitle	= TextWidget.Cast(layoutRoot.FindAnyWidget("DescTitle"));
		m_DescriptionLabel	= MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("DescWidget"));
		m_OkButton			= ButtonWidget.Cast(layoutRoot.FindAnyWidget("OkButton"));
		
		m_DescriptionTitle.SetText(titleText);
		m_DescriptionLabel.SetText(desc);

	    return layoutRoot;
	}
	
	override void OnShow()
	{
		super.OnShow();

		ZenSkillFunctions.SetPlayerControl(false);
	}

	override void OnHide()
	{
		super.OnHide();
		
		if (!GetGame())
			return;

		ZenSkillFunctions.SetPlayerControl(true);
	}
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != MouseState.LEFT)
		{
			return super.OnClick(w, x, y, button);
		}

		if (w == m_OkButton)
		{
			PlaySoundGUI();
			
			GetZenSkillsClientConfig().ShowTutorial = false;
			GetZenSkillsClientConfig().Save();
			
			Close();
			
			return true;
		}
		
		return super.OnClick(w, x, y, button);
	}
	
	void PlaySoundGUI(string sound = "ZenSkillsGUI_Click_SoundSet")
	{		
		if (m_Sound)
		{
			m_Sound.Stop();	
		}		
								
		SoundParams soundParams				= new SoundParams(sound);
		SoundObjectBuilder soundBuilder		= new SoundObjectBuilder(soundParams);
		SoundObject soundObject				= soundBuilder.BuildSoundObject();
		
		soundObject.SetKind(WaveKind.WAVEUI);
		m_Sound = GetGame().GetSoundScene().Play2D(soundObject, soundBuilder);
			
		if (m_Sound)
		{
			m_Sound.Loop(false);
			m_Sound.SetVolume(1);
			m_Sound.Play();
		} 
	}
}
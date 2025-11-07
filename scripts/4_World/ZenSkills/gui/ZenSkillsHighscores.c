class ZenSkillsHighscores extends UIScriptedMenu
{
	static const string LAYOUT_FILE = "ZenSkills/data/gui/layouts/zen_skills_highscores.layout";
	
	static const int HIGHSCORE_REQUEST_DELAY = 1000 * 60 * 10; // 10 minutes

	// Helpers 
	static bool m_RequestedHighscores;
	static int m_LastHighscoreRequest = -1;
	static string m_SelectedSkill = "survival";
	AbstractWave m_Sound;
	
	// Main widgets
	ref TextWidget m_TitleWidget;
	ref MultilineTextWidget m_DescriptionLabel;
	ref TextListboxWidget m_List;
	ref ButtonWidget m_RefreshButton;
	
	// Skill labels
	ref map<string, ref ImageWidget> m_SkillIcons;
	ref map<string, ref TextWidget> m_SkillLabels;

	override Widget Init()
	{
	    layoutRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT_FILE);

		if (!ZenSkillsPlayerDB.RECEIVED_DATA)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Close, 10);
			return layoutRoot;
		}
	
	    m_TitleWidget       = TextWidget.Cast(layoutRoot.FindAnyWidget("TitleWidget"));
		m_DescriptionLabel	= MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("DescriptionLabel"));
		m_List				= TextListboxWidget.Cast(layoutRoot.FindAnyWidget("HighscoresList"));
		m_RefreshButton		= ButtonWidget.Cast(layoutRoot.FindAnyWidget("RefreshButton"));

		m_SkillIcons 		= new map<string, ref ImageWidget>();
		m_SkillLabels		= new map<string, ref TextWidget>();
		
		ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		
		if (!db || !db.Skills)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Close, 10);
			return layoutRoot;
		}

		// Load skill button labels based on what is configured in JSON
		for (int i = 0; i < db.Skills.Count(); i++)
		{
			string skillKey = db.Skills.GetKey(i);
			string labelName = ZenSkillFunctions.FirstLetterUppercase(skillKey);
			string icon = "ZenSkills/data/gui/images/skill_" + labelName + ".edds";
			
			ImageWidget skillIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget(labelName + "ButtonImage"));
			m_SkillIcons.Set(skillKey, skillIcon);
			skillIcon.LoadImageFile(0, icon);
		
			TextWidget skillLabel = TextWidget.Cast(layoutRoot.FindAnyWidget(labelName + "Label"));
			m_SkillLabels.Set(skillKey, skillLabel);
		
			bool loaded = false;
			if (skillLabel != null)
				loaded = true;
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("Loaded label for skill: " + skillKey + " " + loaded);
			#endif
		}
		
		SelectSkill(m_SelectedSkill, false);

	    return layoutRoot;
	}
	
	override void OnShow()
	{
		super.OnShow();
		
		if (m_RequestedHighscores)
		{
			m_RefreshButton.SetText("#STR_ZenSkills_GUI_Wait");
		}
		
		if (m_LastHighscoreRequest == -1)
		{
			RequestRefreshScores();
		}
		
		bool notReady = false;
		
		ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		if (!db || !db.Skills || db.Skills.Count() == 0)
		{
			notReady = true;
		}
		
		ZenSkill testSkill = db.Skills.GetElement(0); 
		if (!testSkill || !testSkill.Perks || !testSkill.Perks.GetElement(0).GetDef())
		{
			notReady = true;
		}
		
		if (notReady)
		{
			Error("DB wasn't ready!");
			Close();
			return;
		}
		
		ZenSkillFunctions.SetPlayerControl(false);
	}

	override void OnHide()
	{
		super.OnHide();

		ZenSkillFunctions.SetPlayerControl(true);
	}
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != MouseState.LEFT)
		{
			return super.OnClick(w, x, y, button);
		}
		
		string name = w.GetName();
		name.ToLower();

		if (name == "closebutton")
		{
			Close();
			return true;
		}
		
		if (name == "survivalbutton")
		{
			SelectSkill("survival");
			return true;
		}

		if (name == "craftingbutton")
		{
			SelectSkill("crafting");
			return true;
		}

		if (name == "huntingbutton")
		{
			SelectSkill("hunting");
			return true;
		}

		if (name == "gatheringbutton")
		{
			SelectSkill("gathering");
			return true;
		}
		
		if (w == m_RefreshButton && !m_RequestedHighscores)
		{
			RequestRefreshScores();
			PlaySoundGUI();
			return true;
		}
		
		return super.OnClick(w, x, y, button);
	}
	
	void RequestRefreshScores()
	{
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Sending request highscores refresh");
		#endif 

		GetRPCManager().SendRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ServerReceive_HighscoreRequest, new Param1<bool>(true), true, null);
		m_RefreshButton.SetText("#STR_ZenSkills_GUI_Wait");
		m_LastHighscoreRequest = GetGame().GetTime();
		m_RequestedHighscores = true;
	}
	
	void ForceUpdateFromServer()
	{
		SelectSkill(m_SelectedSkill, false);
	}
	
	void SelectSkill(string p_skill, bool p_sound = true, bool p_selectPreviousPerk = false)
	{
		if (p_sound)
		{
			PlaySoundGUI();
		}
		
		string description = Widget.TranslateString("#STR_ZenSkills_GUI_TopPlayers");
		description = string.Format(description, GetZenSkillsHighscoresDB().MaxEntries);
		m_DescriptionLabel.SetText(description);

		m_List.ClearItems();
	    m_SelectedSkill = p_skill;

	    ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		if (!db || !db.Skills) 
			return;
		
		ZenSkill skill = db.Skills.Get(m_SelectedSkill);
		if (!skill || !skill.Perks) 
			return;
		
		m_TitleWidget.SetText(skill.GetDef().DisplayName + " #STR_ZenSkills_GUI_Skill");

		foreach (string labelKey, TextWidget label : m_SkillLabels)
		{
			if (labelKey == m_SelectedSkill)
			{
				label.SetColor(ZenSkillsGUI.SELECTED_TEXT_COLOUR);
			}
			else 
			{
				label.SetColor(ZenSkillsGUI.UI_COLOR_WHITE);
			}
		}
		
		// Populate listbox 
		array<ref ZenSkillsHighscoreDef> arrayScores = GetZenSkillsHighscoresDB().Highscores.Get(m_SelectedSkill);
		if (!arrayScores)
		{
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("Failed to get highscores db for skill key: " + m_SelectedSkill + " - might not be received yet.");
			#endif
			
			return;
		}
		
		m_List.ClearItems();
		//string skillString = "#STR_ZenSkills_Name_" + ZenSkillFunctions.FirstLetterUppercase(m_SelectedSkill);
		
		for (int i = 0; i < arrayScores.Count(); i++)
		{
			ZenSkillsHighscoreDef hsDef = arrayScores.Get(i);
			if (!hsDef)
			{
				Error("Failed to get ZenSkillsHighscoreDef for index: " + i);
				continue;
			}
			
			int rankInt = i + 1;
			string rankStr;
			if (rankInt <= 9)
				rankStr = "0" + rankInt.ToString();
			else 
				rankStr = rankInt.ToString();
			
			m_List.AddItem(/*skillString + " | " +*/ rankStr + " > " + hsDef.PlayerName + " - Level: " + hsDef.Level.ToString(), NULL, 0);
		}
	}
	
	override void Update(float timeslice)
	{
		super.Update(timeslice);
		
		if (!GetGame())
			return;
		
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player || !player.IsAlive() || player.IsUnconscious())
		{
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("Closed due to player dying/uncon");
			#endif
			
			Close();
			return;
		}

		if (KeyState(KeyCode.KC_ESCAPE) == 1)
		{
			UIManager ui = GetGame().GetUIManager();
			if (!ui) 
				return;
			
			if (!ui.Back())
			{
				Close();
			}
			
			return;
		}
		
		if (m_RequestedHighscores)
		{
			int countdown = HIGHSCORE_REQUEST_DELAY - (GetGame().GetTime() - m_LastHighscoreRequest);
			
			if (countdown <= 0)
			{
				m_RequestedHighscores = false;
				m_RefreshButton.SetText("#STR_ZenSkills_GUI_Refresh");
			}
			else 
			{
				string countdownText;
				if (countdown > 60000)
				{
					countdownText = (countdown / 1000 / 60).ToString() + "m";
				}
				else 
				{
					countdownText = (countdown / 1000).ToString() + "s";
				}
				
				m_RefreshButton.SetText("#STR_ZenSkills_GUI_Wait " + countdownText);
			}
		}
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
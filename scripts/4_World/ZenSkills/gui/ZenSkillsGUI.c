class ZenSkillsGUI extends UIScriptedMenu
{
	static const string LAYOUT_FILE = "ZenSkills/data/gui/layouts/zen_skills_menu.layout";
	
	static const int TIER_1_PERK_COUNT = 3;
	static const int TIER_2_PERK_COUNT = 2;
	static const int TIER_3_PERK_COUNT = 3;
	static const int TIER_4_PERK_COUNT = 2;
	
	// https://argb-int-calculator.netlify.app
	static const int DISABLED_COLOUR = -6908266;
	static const int SELECTED_COLOUR = -206;
	static const int SELECTED_TEXT_COLOUR = -206;
	static const int TIER_0_COLOUR = -8388608;
	static const int TIER_1_COLOUR = -12696471;
	static const int TIER_2_COLOUR = -12294084;
	static const int TIER_3_COLOUR = -2443707;
	static const int UI_COLOR_WHITE = -1;
	
	// Helpers
	AbstractWave m_Sound;
	ref array<int> m_TierCount;
	static string m_SelectedSkill = "survival";
	static string m_SelectedPerkKey = "none";
	int m_AvailableSkillPoints;
	int m_LastClickTime;
	string m_LastClickWidget;
	int m_LastConfirmDialog;
	
	static bool m_ZenSkillsExpBoostActive;
	static int m_ZenSkillsExpBoostLeftSecs;
	static int m_ZenSkillsExpReceivedTimestamp;

	// Main widgets
	ref TextWidget m_TitleWidget;
	ref TextWidget m_PerkCountLabel;
	ref TextWidget m_SkillPointCountLabel;
	ref TextWidget m_SkillExpBoostLabel;
	ref TextWidget m_SkillExpBoostTimeLabel;
	ref TextWidget m_SkillRightDescWidget;
	ref TextWidget m_SkillRightTitleWidget;
	ref ButtonWidget m_UnlockButton;
	ref ButtonWidget m_ResetButton;
	ref ButtonWidget m_HighscoresButton;
	ref Widget m_ConfirmPanel;
	ref CheckBoxWidget m_ShowEXP;
	
	// Skill labels
	ref map<string, ref ImageWidget> m_SkillIcons;
	ref map<string, ref TextWidget> m_SkillLabels;
	ref map<string, ref TextWidget> m_SkillPerkLabels;
	ref map<string, ref ProgressBarWidget> m_SkillPerkBars;
	
	// Perk tree
	ref map<string, ref ImageWidget> m_PerkTreeIcons;
	ref map<string, ref TextWidget> m_PerkTreeLevels;
	ref map<string, ref ButtonWidget> m_PerkTreeButtons;

	// Left panel
	ref map<string, ref ImageWidget> m_PerkLeftIcons;
	ref map<string, ref TextWidget> m_PerkLeftLevels;
	ref map<string, ref ButtonWidget> m_PerkLeftButtons;
	
	// Confirm dialog
	ref TextWidget m_ConfirmLabel;
	ref ButtonWidget m_ConfirmButton;

	override Widget Init()
	{
	    layoutRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT_FILE);

		if (!ZenSkillsPlayerDB.RECEIVED_DATA)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Close, 10);
			return layoutRoot;
		}
	
	    m_TitleWidget        		= TextWidget.Cast(layoutRoot.FindAnyWidget("TitleWidget"));
	    m_PerkCountLabel    		= TextWidget.Cast(layoutRoot.FindAnyWidget("PerkCountLabel"));
	    m_SkillRightDescWidget		= TextWidget.Cast(layoutRoot.FindAnyWidget("SkillDescWidget"));
		m_SkillRightTitleWidget 	= TextWidget.Cast(layoutRoot.FindAnyWidget("PerkDescTitle"));
		m_SkillPointCountLabel		= TextWidget.Cast(layoutRoot.FindAnyWidget("SkillPointCountLabel"));
		m_SkillExpBoostLabel		= TextWidget.Cast(layoutRoot.FindAnyWidget("ExpBoostLabel"));
		m_SkillExpBoostTimeLabel	= TextWidget.Cast(layoutRoot.FindAnyWidget("ExpBoostTimeLabel"));
	    m_UnlockButton       		= ButtonWidget.Cast(layoutRoot.FindAnyWidget("SubmitButton"));
		m_ResetButton				= ButtonWidget.Cast(layoutRoot.FindAnyWidget("ResetButton"));
		m_HighscoresButton			= ButtonWidget.Cast(layoutRoot.FindAnyWidget("HighscoresButton"));
	    m_ConfirmPanel       		= layoutRoot.FindAnyWidget("ConfirmPanel");
		m_ConfirmLabel 				= TextWidget.Cast(layoutRoot.FindAnyWidget("ConfirmLabel"));
		m_ConfirmButton 			= ButtonWidget.Cast(layoutRoot.FindAnyWidget("ConfirmButton"));
		m_ShowEXP					= CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("ShowExpGainedCB"));
		
		m_HighscoresButton.Show(GetZenSkillsConfig().SharedConfig.AllowHighscores);
		m_SkillExpBoostTimeLabel.Show(false);
		m_SkillExpBoostLabel.Show(false);
		m_ConfirmPanel.Show(false);
	    m_UnlockButton.Show(false);
		m_ResetButton.Show(false);
		
		m_ShowEXP.SetChecked(GetZenSkillsClientConfig().ShowEXP);
	
	    // maps
	    m_PerkTreeIcons     = new map<string, ref ImageWidget>();
	    m_PerkTreeLevels    = new map<string, ref TextWidget>();
	    m_PerkTreeButtons   = new map<string, ref ButtonWidget>();
	
	    m_PerkLeftIcons     = new map<string, ref ImageWidget>();
	    m_PerkLeftLevels    = new map<string, ref TextWidget>();
	    m_PerkLeftButtons   = new map<string, ref ButtonWidget>();
		
		m_SkillIcons 		= new map<string, ref ImageWidget>();
		m_SkillLabels		= new map<string, ref TextWidget>();
		m_SkillPerkLabels	= new map<string, ref TextWidget>();
		m_SkillPerkBars		= new map<string, ref ProgressBarWidget>();
		
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
			
			TextWidget skillPerks = TextWidget.Cast(layoutRoot.FindAnyWidget(labelName + "ButtonPerks"));
			m_SkillPerkLabels.Set(skillKey, skillPerks);
			
			ProgressBarWidget skillBar = ProgressBarWidget.Cast(layoutRoot.FindAnyWidget(labelName + "Progress"));
			m_SkillPerkBars.Set(skillKey, skillBar);
		
			bool loaded = false;
			if (skillLabel != null)
				loaded = true;
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("Loaded label for skill: " + skillKey + " " + loaded);
			#endif
		}
	
	    m_TierCount = new array<int>;
	    m_TierCount.Insert(TIER_1_PERK_COUNT);
	    m_TierCount.Insert(TIER_2_PERK_COUNT);
	    m_TierCount.Insert(TIER_3_PERK_COUNT);
	    m_TierCount.Insert(TIER_4_PERK_COUNT);
		
	    for (int tier = 1; tier <= m_TierCount.Count(); tier++)
	    {
	        int slots = m_TierCount.Get(tier - 1);
	        for (int slot = 1; slot <= slots; slot++)
	        {
	            string key = tier.ToString() + "_" + slot.ToString();
	
	            // Perk tree widgets
	            m_PerkTreeIcons.Set(key,      ImageWidget.Cast(layoutRoot.FindAnyWidget("PerkIcon"  + tier + "_" + slot)));
	            m_PerkTreeLevels.Set(key,     TextWidget.Cast (layoutRoot.FindAnyWidget("PerkLabel" + tier + "_" + slot)));
	            m_PerkTreeButtons.Set(key,    ButtonWidget.Cast(layoutRoot.FindAnyWidget("PerkBtn"  + tier + "_" + slot)));
	
	            // Left panel widgets
	            m_PerkLeftIcons.Set(key,   ImageWidget.Cast(layoutRoot.FindAnyWidget("PerkLeftIcon"  + tier + "_" + slot)));
	            m_PerkLeftLevels.Set(key,  TextWidget.Cast (layoutRoot.FindAnyWidget("PerkLeftLevel" + tier + "_" + slot)));
	            m_PerkLeftButtons.Set(key, ButtonWidget.Cast(layoutRoot.FindAnyWidget("PerkLeftButton" + tier + "_" + slot)));
	        }
	    }
	
	    /*ValidateWidgets(m_PerkTreeIcons);
	    ValidateWidgets(m_PerkTreeLevels);
	    ValidateWidgets(m_PerkTreeButtons);
	    ValidateWidgets(m_PerkLeftIcons);
	    ValidateWidgets(m_PerkLeftLevels);
	    ValidateWidgets(m_PerkLeftButtons);*/

		UpdateSkillPerkLabels();
	    SelectSkill(m_SelectedSkill, false, true);
		UpdateWidgets();
		
	    return layoutRoot;
	}
	
	void UpdateSkillPerkLabels()
	{
		ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		if (!db)
			return;
		
		foreach (string key, ZenSkill skill : db.Skills)
		{
			int perkCount = skill.CountPerksUnused();
			if (perkCount == 0)
			{
				m_SkillPerkLabels.Get(key).SetText("");
			}
			else 
			{
				m_SkillPerkLabels.Get(key).SetText("+" + perkCount);
			}

			m_SkillPerkBars.Get(key).SetCurrent(skill.ProgressToNextPerk());
		}
	}
	
	void UpdateExpBoost()
	{
		m_SkillExpBoostLabel.Show(m_ZenSkillsExpBoostActive);
		m_SkillExpBoostTimeLabel.Show(m_ZenSkillsExpBoostActive);
		
		if (m_ZenSkillsExpBoostActive)
		{
			int secsPassed = (GetGame().GetTime() - m_ZenSkillsExpReceivedTimestamp) / 1000;
			int secsLeft = m_ZenSkillsExpBoostLeftSecs - secsPassed;
			int minutes = Math.Ceil(secsLeft / 60);
			m_SkillExpBoostLabel.SetText("#STR_ZenSkills_GUI_ExpBoost " + GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostMulti + "x");
			m_SkillExpBoostTimeLabel.SetText(minutes.ToString() + " #STR_ZenSkills_GUI_MinutesLeft");
			m_SkillExpBoostTimeLabel.Update();
		}
	}
	
	void ShowTutorial()
	{
		UIManager ui = GetGame().GetUIManager();
		if (!ui) 
			return;
		
		ZenSkillsTutorial ts = ZenSkillsTutorial.Cast(ui.EnterScriptedMenu(ZenSkillConstants.SKILL_TUTORIAL, this));
	}

	override void OnShow()
	{
		super.OnShow();
		
		if (GetZenSkillsClientConfig().ShowTutorial)
		{
			ShowTutorial();
			return;
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
		
		if (!GetGame())
			return;

		ZenSkillFunctions.SetPlayerControl(true);
	}
	
	void RequestUnlockPerk()
	{
		ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		ZenSkill skill = db.Skills.Get(m_SelectedSkill);
		if (!skill || !skill.Perks) 
		{
			Close();
			return;
		}
		
		ZenPerk perk = skill.Perks.Get(m_SelectedPerkKey);
		if (!perk)
		{
			Close();
			return;
		}

		string labelText = "#STR_ZenSkills_GUI_ConfirmPerkUnlock: " + perk.GetDef().DisplayName + "?";
		
		m_ConfirmPanel.Show(true);
		m_UnlockButton.Show(false);
		m_ResetButton.Show(false);
		m_ConfirmLabel.SetText(labelText);
		m_ConfirmButton.Show(true);
		m_LastConfirmDialog = 1;
		PlaySoundGUI();
	}
	
	void RequestResetPerk()
	{
		string labelText = Widget.TranslateString("#STR_ZenSkills_GUI_ResetPerksWarning");
		
		ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		ZenSkill skill = db.Skills.Get(m_SelectedSkill);
		if (!skill || !skill.Perks) 
			return;

		int perkRefund;
		int totalPerksLeft;
		int resultingEXP = GetZenSkillsPlugin().GetResultingRefundPerksEXP(skill, perkRefund, totalPerksLeft);
		
		#ifdef ZENSKILLSDEBUG
		string debugMsg = "skillEXP=" + skill.EXP;
		debugMsg = debugMsg + " resultingEXP=" + resultingEXP;
		debugMsg = debugMsg + " perkRefund=" + perkRefund + " totalPerksLeft=" + totalPerksLeft;
		debugMsg = debugMsg + " EXP_Refund_Modifier=" + skill.GetDef().EXP_Refund_Modifier;
		debugMsg = debugMsg + " EXP_Per_Perk=" + skill.GetDef().EXP_Per_Perk;
		ZenSkillsPrint(debugMsg);
		#endif
		
		labelText = string.Format(labelText, Math.Round(skill.GetDef().EXP_Refund_Modifier * 100).ToString() + "%", perkRefund.ToString(), totalPerksLeft.ToString());

		PlaySoundGUI();
		m_ConfirmPanel.Show(true);
		m_UnlockButton.Show(false);
		m_ResetButton.Show(false);
		m_ConfirmLabel.SetText(labelText);
		m_ConfirmButton.Show(true);
		m_LastConfirmDialog = 2;
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (w == m_ShowEXP)
		{
			GetZenSkillsClientConfig().ShowEXP = m_ShowEXP.IsChecked();
			GetZenSkillsClientConfig().Save();
		}
		
		return super.OnChange(w, x, y, finished);
	}

	// Handles clicks on button widgets
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != MouseState.LEFT)
		{
			return super.OnClick(w, x, y, button);
		}
		
		string name = w.GetName();
		name.ToLower();
		
		if (m_ConfirmPanel.IsVisible())
		{
			if (name == "cancelbutton")
			{
				SelectPerk(m_SelectedPerkKey);
				m_ConfirmPanel.Show(false);
				return true;
			}
	
			if (name == "confirmbutton")
			{
				if (m_LastConfirmDialog == 1)
				{
					SendSelectedPerkUnlockRequest();
				}
				else
				if (m_LastConfirmDialog == 2)
				{
					SendResetPerksRequest();
				}
				
				return true;
			}
			
			// Don't allow interface to be interacted with if confirm dialog is open
			return true;
		}
		
		if (name == "infobutton")
		{
			ShowTutorial();
			return true;
		}
		
		// Couldn't get OnDoubleClick() to work..
		bool doubleClick = false;
		int doubleClickTime = GetGame().GetTime() - m_LastClickTime;

		if (doubleClickTime < 200)
		{
			doubleClick = true;
		}
		else 
		{
			m_LastClickWidget = name;
		}
		
		m_LastClickTime = GetGame().GetTime();

		if (name == "closebutton")
		{
			Close();
			return true;
		}

		if (w == m_UnlockButton)
		{
			RequestUnlockPerk();
			return true;
		}
		
		if (w == m_ResetButton)
		{
			RequestResetPerk();
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

		if (name.Contains("perk"))
		{
			if (doubleClick && m_LastClickWidget == name && m_UnlockButton.IsVisible())
			{
				RequestUnlockPerk();
			}
			else 
			{
				SelectPerk(name);
			}
		}
		
		if (w == m_HighscoresButton)
		{
			OpenHighscores();
			return true;
		}

		return super.OnClick(w, x, y, button);;
	}
	
	void OpenHighscores()
	{
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("Open highscores");
		#endif
		
		UIManager ui = GetGame().GetUIManager();
		if (!ui) 
			return;
	
		UIScriptedMenu top = ui.GetMenu();
		if (top && top.GetID() == ZenSkillConstants.SKILL_HIGHSCORES)
			return;

		ZenSkillsHighscores hs = ZenSkillsHighscores.Cast(ui.EnterScriptedMenu(ZenSkillConstants.SKILL_HIGHSCORES, this));
	}

	void SelectSkill(string p_skill, bool p_sound = true, bool p_selectPreviousPerk = false)
	{
		if (p_sound)
		{
			PlaySoundGUI();
		}

	    m_SelectedSkill = p_skill;

	    ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		if (!db || !db.Skills) 
			return;
		
		ZenSkill skill = db.Skills.Get(m_SelectedSkill);
		if (!skill || !skill.Perks) 
			return;
		
		if (!p_selectPreviousPerk)
	    {
	        m_SelectedPerkKey = "none";
			ResetPerkButtonHighlights();
			UpdateDescriptionText("");
	    }
		
		string icon;
		m_TitleWidget.SetText(skill.GetDef().DisplayName + " #STR_ZenSkills_GUI_Skill");
		m_SkillRightTitleWidget.SetText(skill.GetDef().DisplayName + " #STR_ZenSkills_GUI_Skill");
		
		for (int tier = 1; tier <= m_TierCount.Count(); tier++)
		{
		    int slots = m_TierCount.Get(tier - 1);
			
		    for (int slot = 1; slot <= slots; slot++)
		    {
		        string key = tier.ToString() + "_" + slot.ToString();
		
		        ZenPerk perk = skill.Perks.Get(key);
		        int lvl = 0;
		        if (perk) 
					lvl = perk.Level;
		
		        icon = "ZenSkills/data/gui/images/" + m_SelectedSkill + "/" + key + "_" + lvl + ".paa";
		        ImageWidget iw = m_PerkTreeIcons.Get(key);
		        ImageWidget il = m_PerkLeftIcons.Get(key);
				
		        iw.LoadImageFile(0, icon);
				il.LoadImageFile(0, icon);
		
		        ButtonWidget leftBtn = m_PerkLeftButtons.Get(key);
		        ButtonWidget treeBtn = m_PerkTreeButtons.Get(key);
				
				if (leftBtn) 
				{ 
					string labelText = perk.GetDef().DisplayName;
					if (GetZenSkillsConfig().SharedConfig.DebugMode)
					{
						labelText = labelText + ": " + key;
					}
					
					leftBtn.SetText(labelText); 
				}
				
				TextWidget lvlTxt = m_PerkLeftLevels.Get(key);
		        if (lvlTxt) { lvlTxt.SetText("" + lvl + "/" + perk.GetDef().Rewards.Count()); }
				
				lvlTxt = m_PerkTreeLevels.Get(key);
				if (lvlTxt) { lvlTxt.SetText("" + lvl + "/" + perk.GetDef().Rewards.Count()); }
		
		        if (!db.CanUnlockPerk(m_SelectedSkill, key))
		        {
		            if (treeBtn) { treeBtn.SetColor(DISABLED_COLOUR); }
					if (leftBtn) { leftBtn.SetColor(DISABLED_COLOUR); }
		        }
		        else
		        {
		            // Colour by current level
		            int color = TIER_0_COLOUR;
		            if (lvl == 1) 		{ color = TIER_1_COLOUR; }
		            else if (lvl == 2) 	{ color = TIER_2_COLOUR; }
		            else if (lvl >= 3) 	{ color = TIER_3_COLOUR; }
		            if (treeBtn) 		{ treeBtn.SetColor(color); }
		        }
		    }
			
			m_UnlockButton.Show(false);
		}
		
		foreach (string labelKey, TextWidget label : m_SkillLabels)
		{
			if (labelKey == m_SelectedSkill)
			{
				label.SetColor(SELECTED_TEXT_COLOUR);
			}
			else 
			{
				label.SetColor(UI_COLOR_WHITE);
			}
		}
		
		if (skill.CountPerksUnlocked() > 0 && GetZenSkillsConfig().SharedConfig.AllowResetPerks)
		{
			m_ResetButton.Show(true);
		}
		else 
		{
			m_ResetButton.Show(false);
		}
	
		if (p_selectPreviousPerk)
		{
			SelectPerk(m_SelectedPerkKey, false);
		}
	    
		UpdateSkillPoints();
	}

	void SelectPerk(string widgetName, bool p_sound = true)
	{
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("SelectPerk(" + widgetName + ", p_sound=" + p_sound);
		#endif
		
		if (p_sound)
		{
			PlaySoundGUI();
		}
		
	    string key = widgetName;
	    key.Replace("perkleftbutton", "");
	    key.Replace("perkbtn", "");
		
		m_SelectedPerkKey = "";
		if (key != "none")
		{
			m_SelectedPerkKey = key;
		}
		
		UpdateDescriptionText(m_SelectedPerkKey);
		ResetPerkButtonHighlights(); 
		
	    if (key == "none") 
		{ 
			return; 
		}
	
	    ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
	    if (!db) 
			return;
	
	    if (!db.CanUnlockPerk(m_SelectedSkill, key)) 
		{
			m_UnlockButton.Show(false);
	        int missing = db.CountMissingPerksForUnlock(m_SelectedSkill, key);
			
			#ifdef ZENSKILLSDEBUG
	        ZenSkillsPrint("Locked. Unlock " + missing + " more perk(s) in tier " + (db.GetTierFromKey(key) - 1));
			#endif
			
	        return; // keep it highlighted but don’t enable/unlock
	    }
	
	    // (optional) proceed with any extra UI for unlocked
		if (m_AvailableSkillPoints > 0)
		{
			m_UnlockButton.Show(true);
		}
		else 
		{
			m_UnlockButton.Show(false);
		}
	}
	
	void ResetPerkButtonHighlights()
	{
	    ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
	    if (!db || !db.Skills) 
			return;
	
	    ZenSkill selectedSkill = db.Skills.Get(m_SelectedSkill);
	    if (!selectedSkill || !selectedSkill.Perks) 
			return;
	
	    for (int i = 0; i < m_PerkTreeButtons.Count(); i++)
	    {
	        string key = m_PerkTreeButtons.GetKey(i);
	
	        ZenPerk perk = selectedSkill.Perks.Get(key);
	        int level = perk.Level;
	
	        int color = TIER_0_COLOUR;
	        if (level == 1) 		color = TIER_1_COLOUR;
	        else if (level == 2) 	color = TIER_2_COLOUR;
	        else if (level >= 3) 	color = TIER_3_COLOUR;
	
	        bool canUnlockPerk = db.CanUnlockPerk(m_SelectedSkill, key);
	
	        ButtonWidget treeBtn = m_PerkTreeButtons.Get(key);
	        ButtonWidget leftBtn = m_PerkLeftButtons.Get(key);

	        if (canUnlockPerk) 
			{
	            if (treeBtn) 	{ treeBtn.SetColor(color); }
	            if (leftBtn) 	{ leftBtn.SetTextColor(UI_COLOR_WHITE); }
	        } else 
			{
	            if (treeBtn) 	{ treeBtn.SetColor(DISABLED_COLOUR); }
	            if (leftBtn) 	{ leftBtn.SetTextColor(ARGB(255,160,160,160)); }
	        }
	
	        // -- selection override (works even if locked) --
	        if (m_SelectedPerkKey == key) 
			{
	            if (treeBtn) { treeBtn.SetColor(SELECTED_COLOUR); }
	            if (leftBtn) { leftBtn.SetTextColor(SELECTED_TEXT_COLOUR); }
	        }
	    }

	    UpdateWidgets();
	}
	
	void UpdateDescriptionText(string perkKey = "")
	{
		ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
	    if (!db || !db.Skills) 
			return;
	
	    ZenSkill skill = db.Skills.Get(m_SelectedSkill);
	    if (!skill || !skill.Perks) 
			return;
		
		if (perkKey == "")
		{
			// Update skill description itself
			m_SkillRightDescWidget.SetText(skill.GetDef().Description);
		}
		else 
		{
			// Update selected perk description
		    ZenPerk perk = skill.Perks.Get(perkKey);
		    if (!perk) 
			{
		        m_SkillRightDescWidget.SetText("ERROR: Invalid perk.");
		        UpdateWidgets();
		        return;
		    }
			
			int tier = db.GetTierFromKey(perkKey);
    		int lvl  = perk.Level;
		
		    int totalRewardsCount = 0;
		    if (perk && perk.GetDef().Rewards) 
				totalRewardsCount = perk.GetDef().Rewards.Count();
		
		    // Current reward
	        int curIdx = lvl - 1;
			float currentReward = 0;
	        if (curIdx >= 0 && curIdx < totalRewardsCount) 
			{
	            currentReward = perk.GetDef().Rewards.Get(curIdx);
	        }
			
			if (currentReward == 0)
			{
				currentReward = perk.GetDef().Rewards.Get(0);
			}
			
			// Set title widget 
			m_SkillRightTitleWidget.SetText(perk.GetDef().DisplayName + ":");
		
		    // Build a simple detail string
			string description = Widget.TranslateString(perk.GetDef().Description);
			description = string.Format(description, currentReward.ToString() + perk.GetDef().RewardSymbol) + ".";
		    string txt = description + "\n" + BuildRewardLines(perk);
			m_SkillRightDescWidget.SetText(txt);

		    // Lock info (if any)
		    if (!db.CanUnlockPerk(m_SelectedSkill, perkKey)) 
			{
		        int missing = db.CountMissingPerksForUnlock(m_SelectedSkill, perkKey);
		        if (missing > 0) 
				{
					string missingPerksDesc = Widget.TranslateString("#STR_ZenSkills_GUI_MissingTier");
					missingPerksDesc = string.Format(missingPerksDesc, missing.ToString(), (tier - 1).ToString());
		            txt += "\n#locked: " + missingPerksDesc + ".";
		        } else 
				{
					if (perk.Level >= perk.GetDef().Rewards.Count())
					{
						txt += "\n#locked: #STR_ZenSkills_GUI_MaxLevel.";
					}
					else 
					{
						txt += "\n#locked: #STR_ZenSkills_GUI_NoSkillPoints.";
					}
		        }
		    }
			
			m_SkillRightDescWidget.SetText(txt);
		}
		
		UpdateWidgets();
	}
	
	string BuildRewardLines(ZenPerk perk)
	{
	    string outStr = "~\n";
		
	    int lvl = 0;
	    if (perk) 
			lvl = perk.Level;
	
	    int totalRewardsCount = 0;
	    if (perk && perk.GetDef().Rewards) 
			totalRewardsCount = perk.GetDef().Rewards.Count();
	
	    // Current reward
	    if (lvl <= 0 || totalRewardsCount == 0) 
		{
	        outStr += "#STR_ZenSkills_GUI_CurrentReward: #options_pc_nopause_0\n";
	    } else 
		{
	        int curIdx = lvl - 1;
	        if (curIdx >= 0 && curIdx < totalRewardsCount) 
			{
	            float cur = perk.GetDef().Rewards.Get(curIdx);
	            outStr += "#STR_ZenSkills_GUI_CurrentReward: " + cur.ToString() + perk.GetDef().RewardSymbol + "\n";
	        } else 
			{
	            outStr += "#STR_ZenSkills_GUI_CurrentReward: #options_pc_nopause_0\n";
	        }
	    }
	
	    // Next reward
	    int nextIdx = lvl; // next level index
		if (nextIdx == 0)
			nextIdx = 1;
		
	    if (nextIdx >= 0 && nextIdx < totalRewardsCount) 
		{
	        float next = perk.GetDef().Rewards.Get(nextIdx);
	        outStr += "#STR_ZenSkills_GUI_NextLevelReward: " + next.ToString() + perk.GetDef().RewardSymbol;
	    } else 
		{
	        outStr += "#STR_ZenSkills_GUI_NextLevelReward: (#STR_ZenSkills_GUI_MaxLevel)";
	    }
	
	    return outStr;
	}
	
	void UpdateSkillPoints()
	{
		ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		if (!db || m_SelectedSkill == "")
			return;
		
		ZenSkill currentSkill = db.Skills.Get(m_SelectedSkill);
		if (!currentSkill)
			return;
		
		int perkCount = currentSkill.CountPerksUnlocked();
		int exp = currentSkill.EXP;
		m_AvailableSkillPoints = exp / currentSkill.GetDef().EXP_Per_Perk;
		
		string skillPointsText = m_AvailableSkillPoints.ToString();
		if (m_AvailableSkillPoints > 0)
			skillPointsText = "+" + skillPointsText;
		
		m_SkillPointCountLabel.SetText("#STR_ZenSkills_GUI_SkillPoints: " + skillPointsText);
		m_PerkCountLabel.SetText("#STR_ZenSkills_GUI_Perks: " + perkCount + "/" + currentSkill.GetDef().MaxAllowedPerks);
	}
	
	void UpdateWidgets()
	{
		m_TitleWidget.Update();
		m_PerkCountLabel.Update();
		m_SkillRightDescWidget.Update();
		m_UnlockButton.Update();
		m_ConfirmPanel.Update();
		m_ShowEXP.Update();

		for (int i = 0; i < m_PerkTreeButtons.Count(); i++)
		{
			m_PerkTreeIcons.GetElement(i).Update();
			m_PerkTreeLevels.GetElement(i).Update();
			m_PerkTreeButtons.GetElement(i).Update();
			m_PerkLeftIcons.GetElement(i).Update();
			m_PerkLeftLevels.GetElement(i).Update();
		}
	}
	
	protected void SendSelectedPerkUnlockRequest()
	{
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkills] Request unlock perk for skill=" + m_SelectedSkill + " perk=" + m_SelectedPerkKey);
		#endif
		
		ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		if (!db)
		{
			Error("No skill DB ref!");
			Close();
			return;
		}
		
		if (!db.CanUnlockPerk(m_SelectedSkill, m_SelectedPerkKey))
		{
			// In case of desync (eg. trying to unlock a perk before client receives latest skill EXP update)
			m_ConfirmLabel.SetText("#STR_ZenSkills_GUI_NotEnoughPoints");
			m_ConfirmButton.Show(false);
			return;
		}
		
		m_ConfirmPanel.Show(false);
		SelectSkill(m_SelectedSkill, false, true);
		GetRPCManager().SendRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ServerReceive_PerkUnlock, new Param2<string, string>(m_SelectedSkill, m_SelectedPerkKey), true, null);
	}
	
	protected void SendResetPerksRequest()
	{
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkills] Request reset perks for skill=" + m_SelectedSkill);
		#endif
		
		ZenSkillsPlayerDB db = GetZenSkillsPlugin().GetSkillsDB();
		if (!db)
		{
			Error("No skill DB ref!");
			Close();
			return;
		}

		m_ConfirmPanel.Show(false);
		SelectSkill(m_SelectedSkill, false, true);
		GetRPCManager().SendRPC(ZenSkillConstants.RPC, ZenSkillConstants.RPC_ServerReceive_PerkReset, new Param1<string>(m_SelectedSkill), true, null);
	}
	
	void ForceUpdateFromServer(int playSound)
	{
		if (m_SelectedSkill != "")
		{
			SelectSkill(m_SelectedSkill, false);
		}
		
		if (m_SelectedPerkKey != "")
		{
			SelectPerk(m_SelectedPerkKey, false);
		}

		if (playSound == ZenSkillConstants.SOUND_PERKUNLOCKED)
		{
			PlaySoundGUI("ZenSkillsGUI_PerkUnlocked_SoundSet");
		}
		else
		if (playSound == ZenSkillConstants.SOUND_PERKUNLOCKED_FINAL)
		{
			PlaySoundGUI("ZenSkillsGUI_PerkUnlockedMax_SoundSet");
		}
		else 
		if (playSound == ZenSkillConstants.SOUND_PERKSRESET)
		{
			PlaySoundGUI("ZenSkillsGUI_PerkReset_SoundSet");
		}
		
		UpdateSkillPerkLabels();
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
		
		UpdateExpBoost();

		if (KeyState(KeyCode.KC_ESCAPE) == 1)
		{
			Close();
			return;
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

	private void ErrorMsg(string s)
	{
		DebugMsg("[ZenSkillsGUI] Error - " + s);
		Error("[ZenSkillsGUI] ERROR: " + s);
	}

	private void DebugMsg(string msg)
	{
		GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", msg, ""));
	}
	
	void ValidateWidgets(map<string, ref ImageWidget> widgetList)
	{
		foreach (string key, Widget widget : widgetList)
		{
			if (!widget)
			{
				ErrorMsg("FAILED TO LOAD IMAGE WIDGET: " + key);
			}
		}
	}

	void ValidateWidgets(map<string, ref TextWidget> widgetList)
	{
		foreach (string key, Widget widget : widgetList)
		{
			if (!widget)
			{
				ErrorMsg("FAILED TO LOAD TEXT WIDGET: " + key);
			}
		}
	}

	void ValidateWidgets(map<string, ref ButtonWidget> widgetList)
	{
		foreach (string key, Widget widget : widgetList)
		{
			if (!widget)
			{
				ErrorMsg("FAILED TO LOAD BUTTON WIDGET: " + key);
			}
		}
	}

	void ValidateWidgets(map<string, ref CheckBoxWidget> widgetList)
	{
		foreach (string key, Widget widget : widgetList)
		{
			if (!widget)
			{
				ErrorMsg("FAILED TO LOAD CHECKBOX WIDGET: " + key);
			}
		}
	}
}
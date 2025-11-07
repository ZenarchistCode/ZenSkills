class ZenSkillsNotification extends Managed
{
	int m_Type;               // ZEN_NOTIF_EXP / ZEN_NOTIF_PERK
	float m_Progress;         // EXP only
	string m_Text;            // text for both
	string m_SkillKey;        // PERK only
	int m_PerkCount;          // PERK only
}

class ZenFadeTrack extends Managed
{
	Widget 	m_Widget;
	float 	m_StartAlpha;
	float 	m_TargetA;
	int   	m_DurationMs;
	int   	m_ElapsedMs;

	void Setup(Widget widget, float startAlpha, float targetAlpha, int durMs)
	{
		m_Widget = widget;
		m_StartAlpha = startAlpha;
		m_TargetA = targetAlpha;
		m_DurationMs = Math.Max(1, durMs);
		m_ElapsedMs = 0;
	}

	// returns true when finished
	bool Step(int deltaMs)
	{
		if (!m_Widget) 
			return true;

		m_ElapsedMs = Math.Min(m_DurationMs, m_ElapsedMs + deltaMs);
		float t = m_ElapsedMs / m_DurationMs;
		float a = Math.Lerp(m_StartAlpha, m_TargetA, t);
		m_Widget.SetAlpha(a);
		
		return m_ElapsedMs >= m_DurationMs;
	}
}

class ZenSkillsHUD extends ZenSkillsHUDBase
{
	static const string LAYOUT_FILE = "ZenSkills/data/gui/layouts/zen_skills_hud";
	
	static const float EXP_BAR_INIT_ALPHA = 0.6;
	static const float PERK_PANEL_INIT_ALPHA = 0.9;
	
	static const int ZEN_NOTIFY_DISPLAY_MS 	= 5000; 
	static const int ZEN_FADE_TIME_MS 		= 2000; 

	static const int ZEN_NOTIF_EXP  = 0;
	static const int ZEN_NOTIF_PERK = 1;

	ref array<ref ZenSkillsNotification> m_ZenQueue_EXP;
	ref array<ref ZenSkillsNotification> m_ZenQueue_Notify;
	bool m_ZenIsDisplayingEXP;
	bool m_ZenIsDisplayingNotify;
	bool m_ExpIsFading;
	bool m_PerkIsFading;
	
	ref Widget					m_LayoutRoot;
	ref Widget 					m_NewPerkFrame;
	ref Widget 					m_ExpGainedFrame;

	ref ImageWidget 			m_NewPerkIcon;
	ref MultilineTextWidget 	m_NewPerkLabel;
	ref MultilineTextWidget		m_NewPerkHint;
	ref Widget 					m_NewPerkPanel;

	ref ProgressBarWidget		m_ExpGainedBar;
	ref TextWidget				m_ExpGainedLabel;

	static const int ZEN_FADE_TICK_MS = 33;
	ref array<ref ZenFadeTrack> m_ExpFaders;
	ref array<ref ZenFadeTrack> m_PerkFaders;

	AbstractWave m_Sound;
	
	string GetSkillLayoutFile()
	{
		string leftRight 	= "_right.layout";
		if (GetZenSkillsConfig().SharedConfig.ShowExpHudOnLeft)
			leftRight		= "_left.layout"; 
		
		return LAYOUT_FILE + leftRight;
	}

	void ZenSkillsHUD()
	{
		m_LayoutRoot			= GetGame().GetWorkspace().CreateWidgets(GetSkillLayoutFile());
		m_ExpGainedFrame		= m_LayoutRoot.FindAnyWidget("ExpBarFrame");
		m_NewPerkFrame			= m_LayoutRoot.FindAnyWidget("LevelUpPanel");

		m_NewPerkIcon 			= ImageWidget.Cast(m_LayoutRoot.FindAnyWidget("SkillImage"));
		m_NewPerkLabel			= MultilineTextWidget.Cast(m_LayoutRoot.FindAnyWidget("SkillLabel"));
		m_NewPerkHint			= MultilineTextWidget.Cast(m_LayoutRoot.FindAnyWidget("SkillLabelKeyHint"));
		m_NewPerkPanel			= m_LayoutRoot.FindAnyWidget("SkillPanel");

		m_ExpGainedBar			= ProgressBarWidget.Cast(m_LayoutRoot.FindAnyWidget("ExpBar"));
		m_ExpGainedLabel		= TextWidget.Cast(m_LayoutRoot.FindAnyWidget("ExpLabel"));

		m_ExpFaders  = new array<ref ZenFadeTrack>();
		m_PerkFaders = new array<ref ZenFadeTrack>();

		m_NewPerkPanel.Show(false);
		m_ExpGainedBar.Show(false);
		m_ExpGainedLabel.Show(false);

		m_ZenQueue_EXP = new array<ref ZenSkillsNotification>();
		m_ZenQueue_Notify = new array<ref ZenSkillsNotification>();
		m_ZenIsDisplayingEXP = false;
		m_ZenIsDisplayingNotify = false;
	}

	// Intended to be called whenever a menu is opened or HUD is hidden (inventory, maps, admin tools, ~ key etc)
	override void HideAll()
	{
		if (m_ExpGainedFrame)
		{
			m_ExpGainedFrame.Show(false);
		}

		if (m_NewPerkFrame)
		{
			m_NewPerkFrame.Show(false);
		}
	}

	void SetExpGainedLabel(float progress, string text)
	{
		if (!GetZenSkillsClientConfig().ShowEXP) 
			return;
		
		text.ToUpper();
	
		ZenSkillsNotification n = new ZenSkillsNotification();
		n.m_Type = ZEN_NOTIF_EXP;
		n.m_Progress = progress;
		n.m_Text = text;
		m_ZenQueue_EXP.Insert(n);
	
		// If current EXP is fading, cancel the fade so we switch at once (no wait for fade).
		if (m_ExpIsFading)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(StartFadeDelayedExp);
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnCurrentFinishedEXP);
			StopFadeExp();
			
			// we already passed the slot end; show next immediately
			if (m_ZenIsDisplayingEXP)
			{
				ZenSkillsNotification next = m_ZenQueue_EXP.Get(0);
				m_ZenQueue_EXP.RemoveOrdered(0);
				ZenShowExp(next);
				return;
			}
		}
	
		if (!m_ZenIsDisplayingEXP) 
			ZenTryDisplayNextEXP();
	}
	
	void SetPerkUnlockedLabel(string skillKey, int perkCount, string text)
	{
		if (!GetZenSkillsClientConfig().ShowEXP) 
			return;
		
		if (GetZenSkillsClientConfig().LastPerkNotification.Get(skillKey) == perkCount) 
			return;
		
		string keyName = ZenSkillFunctions.GetInputKeyText(ZenSkillConstants.KEY_INPUT_OPEN_SKILLS_GUI);
		string keyHint = Widget.TranslateString("#STR_ZenSkills_GUI_PressKeySpend");
		keyHint = string.Format(keyHint, keyName);
		m_NewPerkHint.SetText(keyHint);
	
		ZenSkillsNotification notif = new ZenSkillsNotification();
		notif.m_Type = ZEN_NOTIF_PERK;
		notif.m_SkillKey = skillKey;
		notif.m_PerkCount = perkCount;
		notif.m_Text = text;
		m_ZenQueue_Notify.Insert(notif);
		GetZenSkillsClientConfig().LastPerkNotification.Set(skillKey, perkCount);
	
		if (m_PerkIsFading)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(StartFadeDelayedPerk);
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnCurrentFinishedNotify);
			StopFadePerk();
			
			if (m_ZenIsDisplayingNotify)
			{
				ZenSkillsNotification next = m_ZenQueue_Notify.Get(0);
				m_ZenQueue_Notify.RemoveOrdered(0);
				ZenShowPerk(next);
				return;
			}
		}
	
		if (!m_ZenIsDisplayingNotify) 
			ZenTryDisplayNextNotify();
	}
	
	void StartFadeDelayedExp()
	{
		m_ExpIsFading = true;
	
		// build trackers from current alpha -> 0
		m_ExpFaders.Clear();
		ref ZenFadeTrack t1 = new ZenFadeTrack();
		ref ZenFadeTrack t2 = new ZenFadeTrack();
	
		t1.Setup(m_ExpGainedLabel, m_ExpGainedLabel.GetAlpha(), 0.0, ZEN_FADE_TIME_MS);
		t2.Setup(m_ExpGainedBar,   m_ExpGainedBar.GetAlpha(),   0.0, ZEN_FADE_TIME_MS);
	
		m_ExpFaders.Insert(t1);
		m_ExpFaders.Insert(t2);
	
		// start ticking
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ExpFadeTick);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ExpFadeTick, ZEN_FADE_TICK_MS, true);
	}
	
	void StopFadeExp()
	{
		m_ExpIsFading = false;
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ExpFadeTick);
	}
	
	void StartFadeDelayedPerk()
	{
		m_PerkIsFading = true;
	
		m_PerkFaders.Clear();
		ZenFadeTrack p1 = new ZenFadeTrack();
		ZenFadeTrack p2 = new ZenFadeTrack();
		ZenFadeTrack p3 = new ZenFadeTrack();
		ZenFadeTrack p4 = new ZenFadeTrack();
	
		p1.Setup(m_NewPerkPanel, m_NewPerkPanel.GetAlpha(), 0.0, ZEN_FADE_TIME_MS);
		p2.Setup(m_NewPerkIcon,  m_NewPerkIcon.GetAlpha(),  0.0, ZEN_FADE_TIME_MS);
		p3.Setup(m_NewPerkLabel, m_NewPerkLabel.GetAlpha(), 0.0, ZEN_FADE_TIME_MS);
		p4.Setup(m_NewPerkHint, m_NewPerkHint.GetAlpha(),	0.0, ZEN_FADE_TIME_MS);
	
		m_PerkFaders.Insert(p1);
		m_PerkFaders.Insert(p2);
		m_PerkFaders.Insert(p3);
		m_PerkFaders.Insert(p4);
	
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(PerkFadeTick);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(PerkFadeTick, ZEN_FADE_TICK_MS, true);
	}
	
	void StopFadePerk()
	{
		m_PerkIsFading = false;
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(PerkFadeTick);
	}

	void PlaySoundGUI(string sound = "ZenSkillsGUI_PerkNotify_SoundSet")
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

	void HideIcon()
	{
		if (m_ExpGainedFrame)
		{
			m_ExpGainedFrame.Show(false);
		}

		if (m_NewPerkFrame)
		{
			m_NewPerkFrame.Show(false);
		}
	}
	
	protected void ZenTryDisplayNextEXP()
	{
		if (m_ZenIsDisplayingEXP)
			return;
		
		if (!m_ZenQueue_EXP || m_ZenQueue_EXP.Count() == 0)
			return;

		ZenSkillsNotification n = m_ZenQueue_EXP.Get(0);
		m_ZenQueue_EXP.RemoveOrdered(0);
		m_ZenIsDisplayingEXP = true;

		StopFadeExp();
		ZenShowExp(n);
	}
	
	protected void ZenTryDisplayNextNotify()
	{
		if (m_ZenIsDisplayingNotify)
			return;
		
		if (!m_ZenQueue_Notify || m_ZenQueue_Notify.Count() == 0)
			return;

		ZenSkillsNotification n = m_ZenQueue_Notify.Get(0);
		m_ZenQueue_Notify.RemoveOrdered(0);
		m_ZenIsDisplayingNotify = true;

		StopFadePerk();
		ZenShowPerk(n);
	}

	protected void ZenShowExp(ZenSkillsNotification n)
	{
		m_ExpGainedFrame.Show(true);
		m_ExpGainedLabel.SetText(n.m_Text);
		m_ExpGainedLabel.Show(true);
		m_ExpGainedLabel.SetAlpha(1);
		m_ExpGainedBar.SetCurrent(n.m_Progress);
		m_ExpGainedBar.Show(true);
		m_ExpGainedBar.SetAlpha(EXP_BAR_INIT_ALPHA);
	
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(StartFadeDelayedExp);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnCurrentFinishedEXP);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnSlotEndEXP);
		
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ZenOnSlotEndEXP, ZEN_NOTIFY_DISPLAY_MS, false);
	}
	
	protected void ZenShowPerk(ZenSkillsNotification n)
	{
		PlaySoundGUI();
		m_NewPerkFrame.Show(true);
		m_NewPerkIcon.LoadImageFile(0, "ZenSkills/data/gui/images/skill_" + n.m_SkillKey + ".edds");
		m_NewPerkLabel.SetText(n.m_Text);
		m_NewPerkPanel.Show(true);
		m_NewPerkIcon.Show(true);
		m_NewPerkLabel.Show(true);
		m_NewPerkHint.Show(true);
		m_NewPerkPanel.SetAlpha(PERK_PANEL_INIT_ALPHA);
		m_NewPerkIcon.SetAlpha(1);
		m_NewPerkLabel.SetAlpha(1);
		m_NewPerkHint.SetAlpha(1);
	
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(StartFadeDelayedPerk);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnCurrentFinishedNotify);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnSlotEndNotify);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ZenOnSlotEndNotify, ZEN_NOTIFY_DISPLAY_MS, false);
	}
	
	protected void ZenOnSlotEndEXP()
	{
		if (m_ZenQueue_EXP && m_ZenQueue_EXP.Count() > 0)
		{
			StopFadeExp();
			ZenSkillsNotification n = m_ZenQueue_EXP.Get(0);
			m_ZenQueue_EXP.RemoveOrdered(0);
			ZenShowExp(n);
			return;
		}
	
		StartFadeDelayedExp();
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ZenOnCurrentFinishedEXP, ZEN_FADE_TIME_MS, false);
	}
	
	protected void ZenOnSlotEndNotify()
	{
		if (m_ZenQueue_Notify && m_ZenQueue_Notify.Count() > 0)
		{
			StopFadePerk();
			ZenSkillsNotification n = m_ZenQueue_Notify.Get(0);
			m_ZenQueue_Notify.RemoveOrdered(0);
			ZenShowPerk(n);
			return;
		}
	
		StartFadeDelayedPerk();
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ZenOnCurrentFinishedNotify, ZEN_FADE_TIME_MS, false);
	}

	protected void ZenOnCurrentFinishedEXP()
	{
		m_ExpIsFading = false;
		m_ZenIsDisplayingEXP = false;
		if (m_ExpGainedFrame) m_ExpGainedFrame.Show(false);
		if (m_ExpGainedLabel) m_ExpGainedLabel.Show(false);
		if (m_ExpGainedBar)   m_ExpGainedBar.Show(false);
		ZenTryDisplayNextEXP();
	}
	
	protected void ZenOnCurrentFinishedNotify()
	{
		m_PerkIsFading = false;
		m_ZenIsDisplayingNotify = false;
		if (m_NewPerkFrame)  m_NewPerkFrame.Show(false);
		if (m_NewPerkPanel)  m_NewPerkPanel.Show(false);
		if (m_NewPerkIcon)   m_NewPerkIcon.Show(false);
		if (m_NewPerkLabel)  m_NewPerkLabel.Show(false);
		if (m_NewPerkHint)	 m_NewPerkHint.Show(false);
		ZenTryDisplayNextNotify();
	}
	
	void ExpFadeTick()
	{
		bool allDone = true;
		
		for (int i = 0; i < m_ExpFaders.Count(); i++)
		{
			if (!m_ExpFaders.Get(i).Step(ZEN_FADE_TICK_MS))
				allDone = false;
		}
		
		if (allDone)
		{
			StopFadeExp();
			ZenOnCurrentFinishedEXP();
		}
	}
	
	void PerkFadeTick()
	{
		bool allDone = true;
		
		for (int i = 0; i < m_PerkFaders.Count(); i++)
		{
			if (!m_PerkFaders.Get(i).Step(ZEN_FADE_TICK_MS))
				allDone = false;
		}
		
		if (allDone)
		{
			StopFadePerk();
			ZenOnCurrentFinishedNotify();
		}
	}
}

class ZenSkillsNotification extends Managed
{
	int m_Type;               // ZEN_NOTIF_EXP / ZEN_NOTIF_PERK
	float m_Progress;         // EXP only (absolute progress to show)
	string m_Text;            // display text (used for PERK; EXP text is built from stacks)
	string m_SkillKey;        // PERK only
	int m_PerkCount;          // PERK only
}

class ZenFadeTrack extends Managed
{
	ref Widget 	m_Widget;
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

// Internal stack entry for EXP (skill-based, stringtable-safe)
class ZenExpStack extends Managed
{
	string SkillKey;     // e.g. "survival"
	string LabelPrefix;  // "#STR_ZenSkills_Name_Survival #STR_ZenSkills_GUI_ExpGained: +"
	int    Total;        // accumulated EXP for this toast
	float  Progress;     // latest absolute progress for bar
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

	// PERK still uses these
	ref array<ref ZenSkillsNotification> m_ZenQueue_Notify;

	// EXP stacking state (skill-based – no parsing)
	ref map<string, ref ZenExpStack> m_ExpStacks;     // queued stacks by skill
	ref array<string>                m_ExpOrder;      // queued keys in arrival order
	ref ZenExpStack                  m_ExpActive;     // currently visible stack

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

		m_ZenQueue_Notify      = new array<ref ZenSkillsNotification>();
		m_ZenIsDisplayingEXP   = false;
		m_ZenIsDisplayingNotify= false;

		m_ExpStacks = new map<string, ref ZenExpStack>();
		m_ExpOrder  = new array<string>();
		m_ExpActive = null;
	}

	// Intended to be called whenever a menu is opened or HUD is hidden (inventory, maps, admin tools, ~ key etc)
	override void HideAll()
	{
		// keep as-is if you need it later
	}

	// =========================
	// EXP (skill-key based API)
	// =========================

	// Call this from PluginZenSkills with: (selectedSkill, difference, skill.ProgressToNextPerk())
	void SetExpGainedStack(string skillKey, int addAmount, float progress)
	{
		if (!GetZenSkillsClientConfig().ShowEXP) 
			return;

		if (skillKey == "")
			return;

		// cancel any fade if we are about to extend current toast
		if (m_ExpIsFading)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(StartFadeDelayedExp);
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnCurrentFinishedEXP);
			StopFadeExp();
		}

		// Same skill currently showing -> stack, update widgets, reset timer
		if (m_ZenIsDisplayingEXP && m_ExpActive && m_ExpActive.SkillKey == skillKey)
		{
			m_ExpActive.Total   = m_ExpActive.Total + addAmount;
			m_ExpActive.Progress = progress;
			ZS_UpdateActiveExpWidgets();
			ZS_ResetExpSlotTimer();
			return;
		}

		// Otherwise merge into queued stack for this skill
		ZenExpStack st = m_ExpStacks.Get(skillKey);
		if (!st)
		{
			st = new ZenExpStack();
			st.SkillKey    = skillKey;
			st.Total       = addAmount;
			st.Progress    = progress;
			st.LabelPrefix = ZS_BuildLabelPrefix(skillKey); // stringtable-safe
			m_ExpStacks.Insert(skillKey, st);
			m_ExpOrder.Insert(skillKey);
		}
		else
		{
			st.Total    = st.Total + addAmount;
			st.Progress = progress; // latest absolute progress wins
		}

		// If nothing is visible, start cycle
		if (!m_ZenIsDisplayingEXP)
			ZenTryDisplayNextEXP();
	}

	// NOTE: Prefer SetExpGainedStack(skillKey, amount, progress).
	void SetExpGainedLabel(float progress, string text)
	{
		Error("DEPRECATED! Use SetExpGainedStack instead!");
		SetExpGainedStack(text, ZS_ParseLastSignedNumber(text), progress);
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

	// =========================
	// Fade / GUI helpers
	// =========================

	void StartFadeDelayedExp()
	{
		m_ExpIsFading = true;
	
		m_ExpFaders.Clear();
		ref ZenFadeTrack t1 = new ZenFadeTrack();
		ref ZenFadeTrack t2 = new ZenFadeTrack();
	
		t1.Setup(m_ExpGainedLabel, m_ExpGainedLabel.GetAlpha(), 0.0, ZEN_FADE_TIME_MS);
		t2.Setup(m_ExpGainedBar,   m_ExpGainedBar.GetAlpha(),   0.0, ZEN_FADE_TIME_MS);
	
		m_ExpFaders.Insert(t1);
		m_ExpFaders.Insert(t2);
	
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
		p4.Setup(m_NewPerkHint,  m_NewPerkHint.GetAlpha(),	0.0, ZEN_FADE_TIME_MS);
	
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
		if (m_ExpGainedFrame) m_ExpGainedFrame.Show(false);
		if (m_NewPerkFrame)   m_NewPerkFrame.Show(false);
	}
	
	// =========================
	// EXP: show / advance
	// =========================

	protected void ZenTryDisplayNextEXP()
	{
		if (m_ZenIsDisplayingEXP)
			return;
		
		if (!m_ExpOrder || m_ExpOrder.Count() == 0)
			return;

		string key = m_ExpOrder.Get(0);
		m_ExpOrder.RemoveOrdered(0);

		m_ExpActive = m_ExpStacks.Get(key);
		m_ExpStacks.Remove(key);

		m_ZenIsDisplayingEXP = true;
		StopFadeExp();
		ZenShowExp(m_ExpActive);
	}

	protected void ZenShowExp(ZenExpStack st)
	{
		if (!st)
			return;

		m_ExpGainedFrame.Show(true);
		m_ExpGainedLabel.Show(true);
		m_ExpGainedBar.Show(true);

		m_ExpGainedLabel.SetAlpha(1);
		m_ExpGainedBar.SetAlpha(EXP_BAR_INIT_ALPHA);

		string label = st.LabelPrefix + st.Total; // e.g. "#STR... #STR...: +" + 30
		m_ExpGainedLabel.SetText(label);
		m_ExpGainedBar.SetCurrent(st.Progress);

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(StartFadeDelayedExp);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnCurrentFinishedEXP);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnSlotEndEXP);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ZenOnSlotEndEXP, ZEN_NOTIFY_DISPLAY_MS, false);
	}

	protected void ZenOnSlotEndEXP()
	{
		// show next coalesced entry if any
		if (m_ExpOrder && m_ExpOrder.Count() > 0)
		{
			StopFadeExp();

			string key = m_ExpOrder.Get(0);
			m_ExpOrder.RemoveOrdered(0);

			m_ExpActive = m_ExpStacks.Get(key);
			m_ExpStacks.Remove(key);

			ZenShowExp(m_ExpActive);
			return;
		}

		// nothing pending: fade current
		StartFadeDelayedExp();
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ZenOnCurrentFinishedEXP, ZEN_FADE_TIME_MS, false);
	}

	protected void ZenOnCurrentFinishedEXP()
	{
		m_ExpIsFading = false;
		m_ZenIsDisplayingEXP = false;

		m_ExpActive = null;

		if (m_ExpGainedFrame) m_ExpGainedFrame.Show(false);
		if (m_ExpGainedLabel) m_ExpGainedLabel.Show(false);
		if (m_ExpGainedBar)   m_ExpGainedBar.Show(false);

		ZenTryDisplayNextEXP();
	}

	// Update the visible toast after stacking more of the same skill
	protected void ZS_UpdateActiveExpWidgets()
	{
		if (!m_ExpActive)
			return;

		string label = m_ExpActive.LabelPrefix + m_ExpActive.Total;
		m_ExpGainedLabel.SetText(label);
		m_ExpGainedBar.SetCurrent(m_ExpActive.Progress);

		m_ExpGainedFrame.Show(true);
		m_ExpGainedLabel.Show(true);
		m_ExpGainedBar.Show(true);
		m_ExpGainedLabel.SetAlpha(1);
		m_ExpGainedBar.SetAlpha(EXP_BAR_INIT_ALPHA);
	}

	protected void ZS_ResetExpSlotTimer()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ZenOnSlotEndEXP);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ZenOnSlotEndEXP, ZEN_NOTIFY_DISPLAY_MS, false);
	}

	// Build "#STR_ZenSkills_Name_<Skill> #STR_ZenSkills_GUI_ExpGained: +"
	protected string ZS_BuildLabelPrefix(string skillKey)
	{
		string cap = ZenSkillFunctions.FirstLetterUppercase(skillKey);
		string skillString = "#STR_ZenSkills_Name_" + cap;
		string prefix = skillString + " #STR_ZenSkills_GUI_ExpGained: +";
		return prefix;
	}

	// Legacy helper to extract a trailing "+N" from a text (used only by legacy SetExpGainedLabel)
	protected int ZS_ParseLastSignedNumber(string text)
	{
		if (!text || text.Length() == 0)
			return 0;

		// find last '+' or '-' in the string
		int posPlus = text.LastIndexOf("+");
		int posMinus = text.LastIndexOf("-");
		int pos = -1;
		if (posPlus >= 0 && posMinus >= 0)
		{
			if (posPlus > posMinus) pos = posPlus;
			else pos = posMinus;
		}
		else if (posPlus >= 0) pos = posPlus;
		else if (posMinus >= 0) pos = posMinus;

		if (pos < 0)
			return 0;

		string numStr = text.Substring(pos + 1, text.Length() - (pos + 1));
		return numStr.ToInt();
	}

	// =========================
	// PERK: show / advance
	// =========================

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
	
	// =========================
	// Fade ticks
	// =========================

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

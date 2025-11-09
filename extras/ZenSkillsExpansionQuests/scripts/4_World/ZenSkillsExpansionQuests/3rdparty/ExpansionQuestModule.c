#ifdef EXPANSIONMODQUESTS
modded class ExpansionQuestModule 
{
    override protected void CompleteQuest(ExpansionQuest quest, string playerUID, PlayerIdentity identity, bool isAutoComplete = false, ExpansionQuestRewardConfig reward = null, int selectedObjItemIndex = -1)
	{
		super.CompleteQuest(quest, playerUID, identity, isAutoComplete, reward, selectedObjItemIndex);

		PlayerBase player = PlayerBase.Cast(identity.GetPlayer());
		if (!player)
			return;

		int questID = quest.GetQuestConfig().GetID();
        AddEXP_Quest(player, "ExpansionQuest" + questID);
	}

    override void ServerModuleInit()
	{
		super.ServerModuleInit();

        GetZenSkillsEXP_ExpansionQuests();

		if (!m_QuestConfigs)
		{
			return;
		}

		foreach (int questId, ExpansionQuestConfig cfg : m_QuestConfigs)
		{
            ZenSkillsAppendRewardToDescription(questId, cfg);
		}
	}

    void ZenSkillsAppendRewardToDescription(int questId, ExpansionQuestConfig cfg)
    {
        if (!cfg || !cfg.Descriptions || cfg.Descriptions.Count() != 3)
            return;

        string skillName;
        int reward = -1;

        foreach (string skillKey, ZenSkillsEXPDefHolder expDefHolder : GetZenSkillsEXP_ExpansionQuests().ExpDefs)
        {
            foreach (string actionKey, ZenSkillsEXPDef expDef : expDefHolder.ExpDefs)
            {
                actionKey.ToLower();

                if (!actionKey.Contains("expansionquest_"))
                    continue;

                actionKey.Replace("expansionquest_", "");
                int checkId = actionKey.ToInt();

                if (questId == checkId)
                {
                    reward = expDef.EXP;
                    skillName = "#STR_ZenSkills_Name_" + ZenSkillFunctions.FirstLetterUppercase(skillKey);
                }
            } 
        }

        if (reward == -1)
            return;

        string desc1 = cfg.Descriptions.Get(0);
        string desc2 = cfg.Descriptions.Get(1);
        string desc3 = cfg.Descriptions.Get(2);

        desc1 = desc1 + "\n\n[+" + reward + " " + skillName + " EXP]";
        desc2 = desc2 + "\n\n[+" + reward + " " + skillName + " EXP]";
        desc3 = desc3 + "\n\n[+" + reward + " " + skillName + " EXP]";

        cfg.Descriptions.Set(0, desc1);
        cfg.Descriptions.Set(1, desc2);
        cfg.Descriptions.Set(2, desc3);
    }

    void AddEXP_Quest(PlayerBase player, string actionKey, float modifier = 1, bool forceRawEXP = false)
	{
		if (!player || !player.GetIdentity())
			return;
		
		#ifdef EXPANSIONMODAI
		if (player.IsAI())
			return;
		#endif
		
		if (actionKey == "")
			return;

		actionKey.ToLower();
		
		//! NOTE: This approach allows EXP to be awarded for multiple skills per action.
		foreach (string skillKey, ZenSkillsEXPDefHolder expDefHolder : GetZenSkillsEXP_ExpansionQuests().ExpDefs)
		{
			foreach (string actionKeyDef, ZenSkillsEXPDef expDef : expDefHolder.ExpDefs)
			{
				actionKeyDef.ToLower();
				
				if (actionKey == actionKeyDef)
				{
					bool dontApplyNerf = !expDef.ApplyNerf || forceRawEXP;
					bool dontAllowBoosts = !expDef.ApplyBoosts || forceRawEXP;
					int exp = expDef.EXP;
					int expAdjusted = exp;
					
					if (expDef.AllowModifier && !forceRawEXP)
					{
						expAdjusted = Math.Floor(exp * modifier);
						if (expAdjusted <= 0)
							expAdjusted = 1;
					}
					
					#ifdef ZENSKILLSDEBUG 
					string debugMsg = "EXP=" + exp;
					debugMsg = debugMsg + " EXP_Adjusted=" + expAdjusted;
					debugMsg = debugMsg + " modifier=" + modifier;
					debugMsg = debugMsg + "x for skill=" + skillKey;
					debugMsg = debugMsg + " actionKey=" + actionKey;
					debugMsg = debugMsg + " dontApplyNerf=" + dontApplyNerf;
					ZenSkillsPrint(debugMsg);
					#endif
					
					if (expAdjusted > 0)
					{
						GetZenSkillsPlugin().AddEXP(player, skillKey, expAdjusted, actionKey, dontApplyNerf, dontAllowBoosts);
					}
				}
			}
		}
	}
}
#endif
class ActionReadZenSkillBookCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		float time = 10;
		
		if (ZEN_SKILLS_DEBUG_ON)
		{
			time = 0.1;
		}
		
		m_ActionData.m_ActionComponent = new CAContinuousTime(time);
	}
}

class ActionReadZenSkillBook extends ActionContinuousBase
{
	void ActionReadZenSkillBook()
	{
		m_CallbackClass = ActionReadZenSkillBookCB;
		m_CommandUID      = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_SpecialtyWeight = UASoftSkillsWeight.PRECISE_LOW;
	}
	
	override void CreateConditionComponents()  
	{	
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTNone();
	}

	override bool HasTarget()
	{
		return false;
	}

	override string GetText()
	{
		return "#read";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		return true;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		ZenSkills_BookBase book = ZenSkills_BookBase.Cast(action_data.m_MainItem);
		if (!book)
			return;
		
		book.SetHealth(0);
		
		string skillKey = book.GetType();
		skillKey.Replace("ZenSkills_Book_", "");
		skillKey.ToLower();
		
		if (GetZenSkillsConfig().SkillDefs.Get(skillKey) == null)
		{
			Error("Player tried to read a skill book for a skill which doesnt exist: " + book.GetType());
			return;
		}

		GetZenSkillsPlugin().AddEXP_Action(action_data.m_Player, "ReadSkillBook" + skillKey);
	}

	override string GetSoundCategory(ActionData action_data)
	{
		return "Zen_Paper";
	}
}
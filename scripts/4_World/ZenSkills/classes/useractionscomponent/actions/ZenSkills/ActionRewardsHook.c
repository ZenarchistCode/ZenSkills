modded class CAContinuousTime
{
	float GetZenSkillsTimeToComplete()
	{
		return m_DefaultTimeToComplete;
	}
}

modded class ActionBase
{
	void ZenSkillsCheckEXP(ActionData action_data)
	{
		if (!GetGame().IsDedicatedServer())
			return;
		
		CAContinuousTime cat = CAContinuousTime.Cast(action_data.m_ActionComponent);
		float time = -1;
		
		if (cat)
		{
			time = cat.GetZenSkillsTimeToComplete();
		}
		
		if (time < 0)
			time = 1;
		
		ActionWorldCraft awc = ActionWorldCraft.Cast(this);
		if (awc)
		{
			// This is handled by recipe manager
			return;
		}
		
		if (ZEN_SKILLS_DEBUG_ON)
		{
			string msg = "EXP Key: " + ClassName() + " Multi=" + time + "x";
			ZenSkillsPrint(msg);
			ZenSkillFunctions.DebugMessage(msg);
		}
		
		GetZenSkillsPlugin().AddEXP_Action(action_data.m_Player, ClassName(), time);
	}
	
	override void OnEnd(ActionData action_data)
	{
		super.OnEnd(action_data);
		
		if (IsInstant() || IsInherited(AnimatedActionBase))
			return;
		
		ZenSkillsCheckEXP(action_data);
	}
}

modded class ActionContinuousBase
{
	override void OnFinishProgress(ActionData action_data)
	{
		super.OnFinishProgress(action_data);
		
		ZenSkillsCheckEXP(action_data);
	}
}

modded class AnimatedActionBase
{
	override void OnExecute(ActionData action_data)
	{
		super.OnExecute(action_data);
		
		if (IsInherited(ActionContinuousBase))
			return; // ACB will handle this.
		
		ZenSkillsCheckEXP(action_data);
	}
}
modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);

		actions.Insert(ActionInjectZenSkillsSelf);
		actions.Insert(ActionZenSkillsAttachStickToFire);
		actions.Insert(ActionZenSkillsNurturePlant);
		actions.Insert(ActionReadZenSkillBook);		
	}
}

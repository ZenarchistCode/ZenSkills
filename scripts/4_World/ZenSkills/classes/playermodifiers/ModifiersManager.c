enum ZenSkillsModifiers 
{
	MDF_EXPBOOST = 87884201
}

modded class ModifiersManager
{
	override void Init()
	{
        super.Init();

		AddModifier(new ZenSkillsExpBoostMdfr);
	}
}
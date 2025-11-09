class ZenSkills_Injector_ExpBoost extends Inventory_Base
{
	override string GetTooltip()
    {
		string description = ConfigGetString("descriptionShort");

		if (GetZenSkillsConfig() && GetZenSkillsConfig().SharedConfig)
		{
			int minutes = GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostTime / 60;
        	description = string.Format(description, GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostMulti.ToString() + "x", minutes.ToString());
		}
		
        return description;
    }

	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionInjectZenSkillsSelf);
	}
	
	override void OnApply(PlayerBase player)
	{
		if (!player)
			return;

		ModifiersManager mm = player.GetModifiersManager();

		// Try to fetch the running instance
		ZenSkillsExpBoostMdfr mod = ZenSkillsExpBoostMdfr.Cast(mm.GetModifier(ZenSkillsModifiers.MDF_EXPBOOST));

		if (mod && mod.IsActive())
		{
			// EXTEND instead of reset
			mod.ExtendBySeconds(player, GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostTime);

			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[ExpBoost] Reinjected -> extend lifetime by " + GetZenSkillsConfig().SharedConfig.EXP_InjectorBoostTime + "s");
			#endif
			return;
		}

		// Not active -> start fresh
		mm.ActivateModifier(ZenSkillsModifiers.MDF_EXPBOOST);

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ExpBoost] Injected -> activate new boost");
		#endif
	}
}

class ZenSkills_Injector_PerkReset extends Inventory_Base
{
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionInjectZenSkillsSelf);
	}
	
	override void OnApply(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		player.GiveShock(-100);
		GetZenSkillsPlugin().ResetPerks(player.GetIdentity(), GetZenSkillsConfig().SkillDefs.GetKeyArray(), false);
	}
}

class ZenSkills_Injector_AdminDebugTool_MaxEXP extends Inventory_Base
{
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionInjectZenSkillsSelf);
	}
	
	override void OnApply(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		GetZenSkillsPlugin().AddEXP(player, "hunting", 99999999);
		GetZenSkillsPlugin().AddEXP(player, "crafting", 99999999);
		GetZenSkillsPlugin().AddEXP(player, "gathering", 99999999);
		GetZenSkillsPlugin().AddEXP(player, "survival", 99999999);
	}
}

class ZenSkills_Injector_AdminDebugTool_RemoveEXP extends Inventory_Base
{
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionInjectZenSkillsSelf);
	}
	
	override void OnApply(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;

		GetZenSkillsPlugin().ApplyDeathExpPenalty(1, player);
	}
}
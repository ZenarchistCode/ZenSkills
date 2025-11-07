modded class ActionCraftBoneKnifeEnv
{
	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data);
		
		float chance = action_data.m_Player.GetZenPerkRewardPercent01("crafting", ZenPerks.CRAFTING_EXTRA_KNIFE);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("Survival knife perk: chance=" + chance);
		#endif
		
		if (chance <= 0)
			return;
		
		if (Math.RandomFloat01() > chance)
			return;
		
		EntityAI item_ingredient = action_data.m_MainItem;
		EntityAI knife = action_data.m_Player.SpawnEntityOnGroundRaycastDispersed("BoneKnife");
		action_data.m_MainItem.Delete();
		
		MiscGameplayFunctions.TransferItemProperties(item_ingredient, knife);
	}
}
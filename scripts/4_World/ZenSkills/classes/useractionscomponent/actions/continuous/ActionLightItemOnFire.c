modded class ActionLightItemOnFire
{
	override void OnFinishProgressServer(ActionData action_data)
	{
		ItemBase target_item = ItemBase.Cast(action_data.m_Target.GetObject());
		ItemBase item = action_data.m_MainItem;

		if (item && target_item && item.CanIgniteItem(target_item) && item.IsInherited(HandDrillKit))
		{
			ClearActionJuncture(action_data);
			
			float dice = Math.RandomFloat01();
			float successChance = action_data.m_Player.GetZenPerkRewardPercent01("survival", ZenPerks.SURVIVAL_HAND_DRILL);
			if (successChance <= 0)
			{
				successChance = GetZenSkillsConfig().ServerConfig.BaseHandDrillKitSuccess;
			}
			
			bool successful = dice < successChance;
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("Hand drill kit - successChance=" + successChance + " diceRoll=" + dice + " success=" + successful);
			#endif
			
			if (!successful)
			{
				action_data.m_Player.GetSymptomManager().QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_PAIN_LIGHT);

                if (!action_data.m_Player.FindAttachmentBySlotName("Gloves"))
                {
                    PluginLifespan lifespan = PluginLifespan.Cast(GetPlugin(PluginLifespan));
              		lifespan.UpdateBloodyHandsVisibility(action_data.m_Player, true);
                    action_data.m_Player.GetBleedingManagerServer().AttemptAddBleedingSource(0);
                }
				
				item.SetHealth(0);
				action_data.m_Player.GetInventory().DropEntity(InventoryMode.SERVER, action_data.m_Player, item);
			}
			else 
			{
				item.OnIgnitedTarget(target_item);
				target_item.OnIgnitedThis(item);	
				GetZenSkillsPlugin().AddEXP_Action(action_data.m_Player, "HandDrillKitSuccess");
			}
			
			return;
		}
		
		super.OnFinishProgressServer(action_data);
	}
}
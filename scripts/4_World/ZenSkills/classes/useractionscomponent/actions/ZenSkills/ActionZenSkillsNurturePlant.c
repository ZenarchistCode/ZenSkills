class ActionZenSkillsNurturePlantCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(1);
	}
}

class ActionZenSkillsNurturePlant: ActionContinuousBase
{
	void ActionZenSkillsNurturePlant()
	{
		m_CallbackClass 	= ActionZenSkillsNurturePlantCB;
		m_CommandUID 		= DayZPlayerConstants.CMD_ACTIONFB_CRAFTING;
		m_StanceMask 		= DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text 				= "#STR_ZenSkills_GUI_Nurture";
		m_FullBody 			= true;
	}
	
	override void CreateConditionComponents()  
	{	
		m_ConditionTarget	= new CCTNonRuined(UAMaxDistances.DEFAULT);
		m_ConditionItem		= new CCINone();
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (item)
			return false;
		
		float perk = player.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_PLANT_NURTURE);
		if (perk <= 0)
			return false;
		
		Object targetObject = target.GetObject();
		if (targetObject.IsInherited(GardenBase))
		{
			GardenBase gardenBase = GardenBase.Cast(targetObject);
			Slot slot;
			
			array<string> selections = new array<string>;
			gardenBase.GetActionComponentNameList(target.GetComponentIndex(), selections);

			foreach(string selection: selections)
			{
				slot = gardenBase.GetSlotBySelection(selection);
				if (slot)
					break;
			}
			
			if (slot)
			{
				PlantBase plant = PlantBase.Cast(slot.GetPlant());
				if (plant && (plant.IsHarvestable() || plant.IsZenNurtureApplied()))
					return false;
				
				return true;
			}
		}

		return false;
	}
	
	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data);

		if (!action_data.m_Player || !action_data.m_Target || !action_data.m_Target.GetObject())
		{
			return;
		}
		
		float perk = action_data.m_Player.GetZenPerkRewardPercent01("gathering", ZenPerks.GATHERING_PLANT_NURTURE);
		if (perk <= 0)
			return;
		
		Slot slot = GetZenPlantSlot(action_data.m_Target);
		if (!slot)
		{
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("No slot found!");
			#endif
			return;
		}
		
		PlantBase plant = PlantBase.Cast(slot.GetPlant());
		if (!plant)
		{
			#ifdef ZENSKILLSDEBUG 
			ZenSkillsPrint("No plant found!");
			#endif
			return;
		}
		
		plant.ZenApplyNurturePerks(action_data.m_Player);
	}
	
	Slot GetZenPlantSlot(ActionTarget target)
	{
		GardenBase garden_base = GardenBase.Cast(target.GetObject());
		if (!garden_base)
			return null;

		Slot slot;
		
		array<string> selections = new array<string>;
		garden_base.GetActionComponentNameList(target.GetComponentIndex(), selections);
		string selection;

		for (int s = 0; s < selections.Count(); s++)
		{
			selection = selections[s];
			slot = garden_base.GetSlotBySelection(selection);
			if (slot)
				break;
		}
		
		if (slot && slot.GetPlant())
		{
			return slot;	
		}
		
		return null;
	}
}
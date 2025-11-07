class ActionZenSkillsAttachStickToFireCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(0.5);
	}
}

class ActionZenSkillsAttachStickToFire: ActionContinuousBase
{
	void ActionZenSkillsAttachStickToFire()
	{
		m_CallbackClass 	= ActionZenSkillsAttachStickToFireCB;
		m_CommandUID 		= DayZPlayerConstants.CMD_ACTIONFB_CRAFTING;
		m_StanceMask 		= DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text 				= "#place_object";
		m_FullBody 			= true;
	}
	
	override void CreateConditionComponents()  
	{	
		m_ConditionTarget	= new CCTNonRuined(UAMaxDistances.DEFAULT);
		m_ConditionItem		= new CCINonRuined;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		FireplaceBase fireplace_target = FireplaceBase.Cast(target.GetObject());

		if (!fireplace_target || !fireplace_target.IsBaseFireplace())
		{
			return false;
		}

		if (fireplace_target.IsOven() || fireplace_target.IsBarrelWithHoles())
		{
			return false;
		}

		string surfaceType;
		vector stickOri;
		vector stickPos = GetZenSkillsAttachedProjectedStickPos(player, fireplace_target, stickOri);

		GetGame().SurfaceGetType(stickPos[0], stickPos[2], surfaceType);
		if (!GetGame().IsSurfaceDigable(surfaceType))
		{
			return false;
		}

		if (GetGame().IsDedicatedServer())
		{
			return true; // no need to scan for nearby objects on server, let client do that work.
		}

		return CanAddZenAttachedStick(player, fireplace_target.GetPosition(), stickPos);
	}
	
	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data);

		if (!action_data.m_Player || !action_data.m_Target || !action_data.m_Target.GetObject())
		{
			return;
		}

		FireplaceBase fireplace = FireplaceBase.Cast(action_data.m_Target.GetObject());
		if (!fireplace)
		{
			return;
		}

		SharpWoodenStick stick = SharpWoodenStick.Cast(action_data.m_MainItem);

		if (!stick || stick != action_data.m_Player.GetItemInHands())
		{
			return;
		}

		vector projectedStickOri;
		vector projectedStickPos = GetZenSkillsAttachedProjectedStickPos(action_data.m_Player, fireplace, projectedStickOri);

		stick.SetZenSkillsFireplaceData(projectedStickPos, projectedStickOri, fireplace);
		action_data.m_Player.GetHumanInventory().DropEntity(InventoryMode.SERVER, action_data.m_Player, action_data.m_MainItem);

		fireplace.AddZenSkillsAttachedStick(stick);
	}

	static vector GetZenSkillsAttachedProjectedStickPos(notnull PlayerBase player, notnull FireplaceBase fireplace, out vector stickOrientation)
	{
		vector playerPos	= player.GetPosition();
		vector firePos		= fireplace.GetPosition();
		vector towardsFire	= vector.Direction(playerPos, firePos).Normalized();

		// Offset stick slightly toward fire and angle down a bit
		vector stickOffset = (towardsFire * 0.5);
		vector stickPos = (firePos - stickOffset) + "0 0.2 0";

		float yaw = towardsFire.VectorToAngles()[0];
		yaw = Math.ModFloat((yaw + 180), 360);
		if (yaw < 0)
			yaw += 360;

		stickOrientation[0] = yaw;
		stickOrientation[1] = 35;
		stickOrientation[2] = 0;

		return stickPos;
	}

	// Limit how many sticks can be placed around a fire to within ~0.75m of each other
	// This is so cooking pots etc are still valuable, but also because it looks stupid
	// to have fifteen sticks around a fire.
	static bool CanAddZenAttachedStick(notnull PlayerBase player, vector firePos, vector stickProjectedPos)
	{
		int stickCount = 0;
		stickProjectedPos[1] = 0;
		vector otherStickPos;
		array<Object> nearest_objects = new array<Object>;
		GetGame().GetObjectsAtPosition3D(firePos, 2, nearest_objects, NULL);

		foreach (Object obj : nearest_objects)
		{
			if (obj.IsInherited(SharpWoodenStick))
			{
				otherStickPos = obj.GetPosition();
				otherStickPos[1] = 0;

				// Don't allow placing a stick too close to another stick
				if (vector.Distance(stickProjectedPos, otherStickPos) < 0.75)
				{
					return false;
				}
				else 
				{
					stickCount++;
				}
			}
		}

		int allowedStickCount = (int)player.GetZenPerkReward("survival", ZenPerks.SURVIVAL_FIRE_STICKS);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("Fireplace sticks - allowedStickCount=" + allowedStickCount + " currentSticks=" + stickCount);
		#endif
		
		if (allowedStickCount <= 0)
			return false;

		return stickCount < allowedStickCount;
	}
}
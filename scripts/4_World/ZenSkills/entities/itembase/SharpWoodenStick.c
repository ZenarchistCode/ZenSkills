modded class SharpWoodenStick
{
	protected bool m_ZenSkillsFireplaceActionTrigger;
	protected vector m_ZenSkillsFireplaceSpawnPosition;
	protected vector m_ZenSkillsFireplaceSpawnOrientation;
	protected FireplaceBase m_ZenSkillsFireplaceDaddy;

	void SharpWoodenStick()
	{
		m_ZenSkillsFireplaceActionTrigger = false;
	}

	FireplaceBase GetZenSkillsFireplaceDaddy()
	{
		return m_ZenSkillsFireplaceDaddy;
	}

	// Push stick up and backwards
	void ZenSkillsDropTheStick(bool restart = false)
	{
		vector impulse = Vector(0, 5, -1);

		if (restart)
			impulse = Vector(0, 10, -10);

		ThrowPhysically(null, impulse);
		ZenSkillsStopCookingSound();
	}

	void SetZenSkillsFireplaceData(vector pos, vector ori, FireplaceBase fire)
	{
		m_ZenSkillsFireplaceSpawnPosition = pos;
		m_ZenSkillsFireplaceSpawnOrientation = ori;
		m_ZenSkillsFireplaceDaddy = fire;

		if (m_ZenSkillsFireplaceDaddy && m_ZenSkillsFireplaceSpawnPosition != vector.Zero && m_ZenSkillsFireplaceSpawnOrientation != vector.Zero)
		{
			m_ZenSkillsFireplaceActionTrigger = true;
		}
		else 
		{
			m_ZenSkillsFireplaceActionTrigger = false;
			ZenSkillsStopCookingSound();
		}
	}

	void ZenSkillsStandUpByFire()
	{
		if (!m_ZenSkillsFireplaceSpawnPosition || !m_ZenSkillsFireplaceSpawnOrientation)
			return;

		if (m_ZenSkillsFireplaceSpawnPosition == vector.Zero || m_ZenSkillsFireplaceSpawnOrientation == vector.Zero)
			return;

		SetPosition(m_ZenSkillsFireplaceSpawnPosition);
		SetOrientation(m_ZenSkillsFireplaceSpawnOrientation);

		m_ZenSkillsFireplaceSpawnPosition = vector.Zero;
		m_ZenSkillsFireplaceSpawnOrientation = vector.Zero;
	}

	void ZenSkillsStopCookingSound()
	{
		Edible_Base item_on_stick = Edible_Base.Cast(GetAttachmentByType(Edible_Base));
		if (item_on_stick)
		{
			item_on_stick.MakeSoundsOnClient(false);
		}
	}

	//! VANILLA FUNCTIONS

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();

		vector ori = GetOrientation();

		//! TODO: This is a bit hacky, find a better way?
		// If on server restart ori y axis is tilted within range of my fire attachment range, assume we 
		// were placed by a fire and drop to ground if we have no attachment or our attachment is food. The reason 
		// I check for food specifically is because some mods use things like Human Skulls attached to sticks etc and 
		// allow players to place them similarly to how I place food sticks, so don't wanna drop them accidentally.
		// I do this to avoid confusion around server restarts - without saving to persistence or getting janky with
		// GetObjectAt checks on a server restart, it's not easy to know if a stick was 'placed' at a fire or not. So by
		// "dropping" the stick by the fire on a restart it makes it obvious it needs to be re-placed at the fire to cook.
		if (ori[1] >= 20 && ori[1] <= 40)
		{
			EntityAI item_on_stick = FindAttachmentBySlotName("ingredient");
			if (!item_on_stick || item_on_stick.IsInherited(Edible_Base))
			{
				ZenSkillsDropTheStick(true);
			}			
		}
	}

	override void OnInventoryExit(Man player)
	{
		super.OnInventoryExit(player);

		if (!GetGame().IsDedicatedServer())
			return;
		
		if (m_ZenSkillsFireplaceActionTrigger && player != NULL)
		{
			m_ZenSkillsFireplaceActionTrigger = false;
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ZenSkillsStandUpByFire, 50, false);
		}
	}

	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);

		if (!GetGame().IsDedicatedServer())
			return;

		if (oldLoc.GetParent() != NULL)
			return; // We're only interested in stick being moved from ground (null parent)

		if (m_ZenSkillsFireplaceDaddy != NULL)
		{
			m_ZenSkillsFireplaceDaddy.RemoveZenSkillsAttachedStick(this);
			SetZenSkillsFireplaceData(vector.Zero, vector.Zero, NULL);
		}
	}

	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionZenSkillsAttachStickToFire);
	}
}
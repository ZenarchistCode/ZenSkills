modded class FireplaceBase
{
	protected ref array<SharpWoodenStick> m_ZenSkillsAttachedSticks;

	override void Heating()
	{
		super.Heating();

		if (!IsBurning() || !m_ZenSkillsAttachedSticks || m_ZenSkillsAttachedSticks.Count() == 0)
			return;

		CookZenSkillsSticksAttached();
	}

	protected void CookZenSkillsSticksAttached()
	{
		SharpWoodenStick stick;

		for (int i = m_ZenSkillsAttachedSticks.Count() - 1; i >= 0; i--)
		{
			stick = SharpWoodenStick.Cast(m_ZenSkillsAttachedSticks.Get(i));
			if (!stick || stick.IsRuined() || vector.Distance(stick.GetPosition(), GetPosition()) > 2)
			{
				if (stick.IsRuined())
				{
					stick.ZenSkillsDropTheStick();
				}

				m_ZenSkillsAttachedSticks.Remove(i);
				continue;
			}

			stick.DecreaseHealth(GameConstants.FIRE_ATTACHMENT_DAMAGE_PER_SECOND * 5, false);

			Edible_Base item_on_stick = Edible_Base.Cast(stick.GetAttachmentByType(Edible_Base));
			if (!item_on_stick || !item_on_stick.CanBeCookedOnStick())
			{
				continue;
			}

			GetCookingProcess().CookOnStick(item_on_stick, 1);
		}
	}

	void AddZenSkillsAttachedStick(SharpWoodenStick stick)
	{
		if (!m_ZenSkillsAttachedSticks)
		{
			m_ZenSkillsAttachedSticks = new array<SharpWoodenStick>;
		}

		m_ZenSkillsAttachedSticks.Insert(stick);
	}

	void RemoveZenSkillsAttachedStick(SharpWoodenStick stick)
	{
		if (!m_ZenSkillsAttachedSticks)
		{
			return;
		}

		m_ZenSkillsAttachedSticks.RemoveItem(stick);
	}
}
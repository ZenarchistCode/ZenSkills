modded class Clothing
{
	protected bool m_ZenSkillsLastDamageWasHuman;
	
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		
		if (!source)
			return;

		m_ZenSkillsLastDamageWasHuman = source.IsMan();
		float damage = damageResult.GetDamage("", "");
		
		if (damage <= 0)
			return;

		PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
		if (!player)
			return;

		float perkDurabilityBoost = player.GetZenPerkRewardPercent01("survival", ZenPerks.SURVIVAL_CLOTHES_DURABILITY);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("Clothing " + GetType() + ": perkDurabilityBoost=" + perkDurabilityBoost);
		float oldHP = GetHealth();
		#endif
		
		if (perkDurabilityBoost > 0)
		{
			float refund = damage * perkDurabilityBoost;
			
			if (!IsRuined())
			{
				AddHealth("", "", refund);
			}
			
			#ifdef ZENSKILLSDEBUG
			string debugMsg = "[CLOTHING PERK] " + GetType();
			debugMsg = debugMsg + " hit by " + source;
			debugMsg = debugMsg + " - dmg=" + damage;
			debugMsg = debugMsg + " refundPct=" + perkDurabilityBoost;
			debugMsg = debugMsg + " refundHP=" + refund;
			debugMsg = debugMsg + " newHP=" + GetHealth();
			debugMsg = debugMsg + " oldHP=" + oldHP;
			ZenSkillsPrint(debugMsg);
			#endif
		}		
	}
	
    override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		
		if (!GetGame().IsDedicatedServer())
			return;
		
		if (!IsInitialized() || m_ZenSkillsLastDamageWasHuman)
			return;

		if (oldLevel <= GameConstants.STATE_DAMAGED && newLevel == GameConstants.STATE_RUINED)
		{
			PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
			if (!player)
				return;

			float perkDurabilityBoost = player.GetZenPerkRewardPercent01("survival", ZenPerks.SURVIVAL_CLOTHES_DURABILITY);
	
			if (perkDurabilityBoost > 0)
			{
				// Clothing has X% chance to be non-ruined if new state is ruined and previous state was DAMAGED or better.
				if (Math.RandomFloat01() < perkDurabilityBoost)
				{
					AddHealth("", "", 1);
					
					#ifdef ZENSKILLSDEBUG
					ZenSkillsPrint(GetType() + " saved from ruined! Chance was=" + perkDurabilityBoost);
					#endif
				}
				else 
				{
					#ifdef ZENSKILLSDEBUG
					ZenSkillsPrint(GetType() + " failed to save from ruined. Chance was=" + perkDurabilityBoost);
					#endif
				}
			}
		}
    }
}
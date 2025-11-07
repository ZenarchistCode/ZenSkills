modded class HealthRegenMdfr
{
	override void OnTick(PlayerBase player, float deltaT)
	{	
		super.OnTick(player, deltaT);
		
		if (!player.IsAlive())
			return;

		//Leg regen when legs are NOT BROKEN
		if (player.GetBrokenLegs() != eBrokenLegs.NO_BROKEN_LEGS)
		{
			// 0..1 (e.g., 0.25 = 25% faster healing for broken legs)
			float pct = player.GetZenPerkRewardPercent01("survival", ZenPerks.SURVIVAL_BROKEN_LEG_HEALING);
			if (pct <= 0) 
				return;

			// Vanilla uses per-tick constants for legs (not scaled by deltaT)
			float bonus = PlayerConstants.LEG_HEALTH_REGEN_BROKEN * pct;

			player.AddHealth("RightLeg", "Health", bonus);
			player.AddHealth("RightFoot","Health", bonus);
			player.AddHealth("LeftLeg",  "Health", bonus);
			player.AddHealth("LeftFoot", "Health", bonus);
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("LEG HEALING PERK: Add bonus HP to legs: " + bonus);
			#endif
		}
	}
}
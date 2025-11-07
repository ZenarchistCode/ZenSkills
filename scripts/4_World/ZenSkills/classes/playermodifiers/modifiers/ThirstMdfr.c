modded class ThirstMdfr
{
	override void OnTick(PlayerBase player, float deltaT)
	{
		// Snapshot BEFORE vanilla tick
		PlayerStat<float> waterStat = player.GetStatWater();
		float before = waterStat.Get();

		// Let vanilla compute and apply drain, sounds, damage, etc.
		super.OnTick(player, deltaT);

		// Check if refund is necessary
		if (!player.IsAlive()) 
			return;

		// 0..1, e.g. 0.15 = 15% slower drain
		float pct = player.GetZenPerkRewardPercent01("survival", ZenPerks.SURVIVAL_METABOLISM);
		if (pct <= 0) 
			return;

		float after  = waterStat.Get();
		float decay  = before - after;     	// positive only if water decreased this tick
		if (decay <= 0) 					// gained/unchanged: nothing to refund
			return;          

		// Refund a fraction of this tick's drain
		float refund = decay * pct;
		float newVal = Math.Clamp(after + refund, 0, PlayerConstants.SL_WATER_MAX);
		waterStat.Set(newVal);

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ThirstPerk] pct=" + pct + " before=" + before + " after=" + after + " decay=" + decay + " refund=" + refund + " new=" + newVal);
		#endif
	}
}

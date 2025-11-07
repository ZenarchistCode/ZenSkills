modded class Environment
{
    override protected void ProcessHeatBuffer(EnvironmentSnapshotData data)
    {
        #ifdef EXPANSIONMODAI
		if (m_Player && m_Player.IsAI())
		{
			super.ProcessHeatBuffer(data);
			return;
		}
		#endif

        // 1) Snapshot BEFORE vanilla
        PlayerStat<float> heatBuffer = m_Player.GetStatHeatBuffer();
        float before = heatBuffer.Get();

        // 2) Let vanilla do its thing
        super.ProcessHeatBuffer(data);

        // 3) Fetch perk percent (0..1). If none, stop.
        if (!m_Player) 
			return;

        float pct = m_Player.GetZenPerkRewardPercent01("survival", ZenPerks.SURVIVAL_HEAT_BUFF); // e.g. 0.25 = 25% slower decay
        if (pct <= 0) 
			return;

        // 4) Compute vanilla decay this tick
        float after = heatBuffer.Get();
        float decay = before - after;		// positive only if it actually decreased
        if (decay <= 0) 
			return;            				// increasing or unchanged: do nothing

        // 5) Add back a fraction of that decay → slows the reduction by pct
        float refund = decay * pct;
        float newVal = after + refund;
        if (newVal < 0) 
			newVal = 0;

        heatBuffer.Set(newVal);

		#ifdef ZENSKILLSDEBUG
        ZenSkillsPrint("[HB] pct=" + pct + " before=" + before + " after=" + after + " decay=" + decay + " refund=" + refund + " newVal=" + newVal);
		#endif
    }
}
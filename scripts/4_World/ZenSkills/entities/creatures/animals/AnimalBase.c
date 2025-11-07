modded class AnimalBase
{
    #ifndef SERVER
	// CLIENT-SIDE ONLY ANIMAL TRACKER CODE:
	
    protected vector m_ZenAnimalTrackLastPos;
    protected float m_ZenAnimalTrackAccumTime;
    protected bool m_ZenAnimalTrackInitDone;
	protected static int m_ZenAnimalTrackLastPerkCheck;

    // Sampling
    protected float  m_ZenAnimalTrackScanSecs  				= 0.4;   	// seconds between movement samples
    protected float  m_ZenAnimalTrackStepMeters     		= 5.0;   	// spawn breadcrumb every ~5m moved
    protected static float  m_ZenAnimalTrackMaxRangeMeters 	= 0;		// only track/reveal near player

    // Reveal cadence
	static const float ZEN_REVEAL_INTERVAL 					= 1.0; 		// seconds between bursts
    protected float m_ZenAnimalTrackRevealAccum 			= 0.0;     	// accumulates while holding breath
    protected int m_ZenAnimalTrackBurstToken 				= 0;        // cancels queued callbacks on release
	protected bool m_ZenAnimalTrackEnabled					= false;	// whether or not tracking has been triggered
	
	// How long to show tracking after breath releases
	protected bool  m_ZenAnimalTrackPrevHolding 			= false;	// was holding breath on previous tick?
	protected float m_ZenAnimalTrackAfterReleaseLeft 		= 0.0;		// how long the after-effect has (delta time)
	protected float m_ZenAnimalTrackHoldDurationAccum 		= 0.0;		// how long the after-effect will last
	static const float ZEN_REVEAL_LINGER_BASE   			= 1.0; 		// sec, minimum linger
	static const float ZEN_REVEAL_LINGER_PER_S  			= 1.0; 		// +sec of linger per 1s held
	static const float ZEN_REVEAL_LINGER_MAX    			= 10.0; 	// max clamp

    // Burst shaping
    protected const int   ZEN_BURST_MAX_PLAY 				= 20; 		// play closest ~X per burst
    protected const int   ZEN_BURST_STEP_MS  				= 100; 		// delay between plays in ms

    // Breadcrumbs (chronological: oldest -> newest)
    static const int ZEN_ANIMAL_TRACKING_MAX_CRUMBS = 200;				// max number of positions saved
    protected ref array<vector> m_ZenAnimalTrackBreadcrumbs;			// vector breadcrumbs of animal's movement
	
	// FPS throttling
	protected float m_ZenAnimalTrackOuterTickAccum;						// throttle delta
	static const float ZEN_ANIMAL_TRACKING_TICK = 0.2;					// run checks every 200ms

    // Particle choice
    static const int ZENSKILLS_TRACKING_PARTICLE_DIRT = ParticleList.IMPACT_SAND_ENTER;
    //static const int ZENSKILLS_TRACKING_PARTICLE_SNOW = ParticleList.IMPACT_SNOW_ENTER;
	
	#ifdef ZENSKILLSDEBUG 
	float m_ZenSkillDebugDelta;
	#endif

    override void EEInit()
    {
        super.EEInit();
		
		#ifdef ZENSKILLSDEBUG
		if (GetGame().GetPlayer())
			ZenSkillsPrint("INIT ANIMAL TRACKER: " + GetType() + " @ " + GetPosition() + " dist=" + vector.Distance(GetPosition(), GetGame().GetPlayer().GetPosition()));
		#endif 
		
        m_ZenAnimalTrackBreadcrumbs = new array<vector>();
        m_ZenAnimalTrackLastPos = GetPosition();
		UpdateZenSkillTrackingPerk();
		
		m_ZenAnimalTrackInitDone = true;
    }
	
	override void EEHitByRemote(int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos)
	{
		super.EEHitByRemote(damageType, source, component, dmgZone, ammo, modelPos);
		
		#ifdef ZENSKILLSDEBUG
		// todo: only track animals which were injured by the player?
		ZenSkillsPrint("ANIMAL TRACKING: Hit animal: " + GetType());
		m_ZenAnimalTrackEnabled = true;
		#endif
	}
	
	override void EEDelete(EntityAI parent)
	{
	    super.EEDelete(parent);
		
	    m_ZenAnimalTrackBurstToken++;   // cancels queued callbacks
	}
	
	void UpdateZenSkillTrackingPerk()
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player) 
			return;

		m_ZenAnimalTrackLastPerkCheck = GetGame().GetTime();
		m_ZenAnimalTrackMaxRangeMeters = player.GetZenPerkReward("hunting", ZenPerks.HUNTING_ANIMAL_TRACKING);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("ANIMAL TRACKING: Perk reward track distance=" + m_ZenAnimalTrackMaxRangeMeters);
		#endif
	}

    override void EOnFrame(IEntity other, float timeSlice)
    {
        super.EOnFrame(other, timeSlice);
		
		m_ZenAnimalTrackOuterTickAccum += timeSlice;
	    if (m_ZenAnimalTrackOuterTickAccum < ZEN_ANIMAL_TRACKING_TICK)
	        return;

	    float dt = m_ZenAnimalTrackOuterTickAccum;
	    m_ZenAnimalTrackOuterTickAccum = 0.0;
		
        UpdateZenSkillsAnimalTracker(dt);
    }

    void UpdateZenSkillsAnimalTracker(float timeSlice)
    {
        if (!m_ZenAnimalTrackInitDone) 
			return;

        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.IsAlive()) 
			return;
		
		vector playerPos = player.GetPosition();
		vector curPos = GetPosition();
		float activateDist;
		
		if (!m_ZenAnimalTrackEnabled)
		{
			activateDist = ZenSkills_DistanceSq(player.GetPosition(), GetPosition());
			if (activateDist > 100 * 100)
				return;
		}
		
		#ifdef ZENSKILLSDEBUG
		m_ZenSkillDebugDelta += timeSlice;
		if (m_ZenSkillDebugDelta > 5)
		{
			m_ZenSkillDebugDelta = 0;
			ZenSkillsPrint("ANIMAL TRACKING: " + GetType() + " @ " + GetPosition() + ": m_ZenAnimalTrackBreadcrumbs count=" + m_ZenAnimalTrackBreadcrumbs.Count());
		}
		
		if (!m_ZenAnimalTrackEnabled)
		{
			m_ZenAnimalTrackEnabled = true;
			ZenSkillsPrint("Animal Tracker: Animal tracking activated for " + GetType() + " @ " + GetPosition() + " distSq=" + activateDist);
		}
		#else
		m_ZenAnimalTrackEnabled = true;
		#endif

		bool holdingBreath = player.IsTryingHoldBreath();
		int  timeTick = GetGame().GetTime();
		
		// refresh perk only while actually holding
		if (holdingBreath && (timeTick - m_ZenAnimalTrackLastPerkCheck) >= 60000)
		{
		    UpdateZenSkillTrackingPerk();
		    m_ZenAnimalTrackLastPerkCheck = timeTick;
		}
		
		// accumulate hold duration
		if (holdingBreath)
		{
		    // start of hold -> reset accumulator
		    if (!m_ZenAnimalTrackPrevHolding) 
				m_ZenAnimalTrackHoldDurationAccum = 0.0;
			
		    m_ZenAnimalTrackHoldDurationAccum += timeSlice;
		}
		else
		{
		    // release edge -> compute linger from how long breath was held
		    if (m_ZenAnimalTrackPrevHolding)
		    {
		        float linger = ZEN_REVEAL_LINGER_BASE + ZEN_REVEAL_LINGER_PER_S * m_ZenAnimalTrackHoldDurationAccum;
		        if (linger > ZEN_REVEAL_LINGER_MAX) 
					linger = ZEN_REVEAL_LINGER_MAX;
				
		        m_ZenAnimalTrackAfterReleaseLeft = linger;
		        m_ZenAnimalTrackHoldDurationAccum = 0.0;
		    }
		
		    // countdown linger window
		    if (m_ZenAnimalTrackAfterReleaseLeft > 0.0)
		    {
		        m_ZenAnimalTrackAfterReleaseLeft -= timeSlice;
		        if (m_ZenAnimalTrackAfterReleaseLeft < 0.0) 
					m_ZenAnimalTrackAfterReleaseLeft = 0.0;
		    }
		}
		
		// reveal is active if holding OR still in linger
		bool revealActive = holdingBreath || (m_ZenAnimalTrackAfterReleaseLeft > 0.0);
		
		// bursts
		if (revealActive)
		{
		    m_ZenAnimalTrackRevealAccum += timeSlice;
		    if (m_ZenAnimalTrackRevealAccum >= ZEN_REVEAL_INTERVAL)
		    {
		        m_ZenAnimalTrackRevealAccum = 0.0;
		        EmitBreadcrumbsSequential(player, ZEN_BURST_STEP_MS);
		    }
		}
		else
		{
		    m_ZenAnimalTrackRevealAccum = 0.0;
		    m_ZenAnimalTrackBurstToken++; // cancel any queued bursts after linger ends
		}
		
		m_ZenAnimalTrackPrevHolding = holdingBreath;

        // Only store movement when player is within range
        float distToPlayer = vector.Distance(playerPos, curPos);
        if (distToPlayer > m_ZenAnimalTrackMaxRangeMeters) 
			return;

        m_ZenAnimalTrackAccumTime += timeSlice;
        if (m_ZenAnimalTrackAccumTime < m_ZenAnimalTrackScanSecs) 
			return;
		
        m_ZenAnimalTrackAccumTime = 0.0;

        float moved = vector.Distance(m_ZenAnimalTrackLastPos, curPos);
        if (moved < m_ZenAnimalTrackStepMeters) 
			return;

        int steps = Math.Max(1, Math.Floor(moved / m_ZenAnimalTrackStepMeters));
        for (int i = 1; i <= steps; i++)
        {
            float t = (i * m_ZenAnimalTrackStepMeters) / moved;
            if (t > 1.0) 
				t = 1.0;
			
            vector p = m_ZenAnimalTrackLastPos + (curPos - m_ZenAnimalTrackLastPos) * t;
            SpawnZenBreadcrumb(p);
        }

        m_ZenAnimalTrackLastPos = curPos;
    }

    // Select closest ~X visible breadcrumbs and play them with slight delays to show direction of movement
    protected void EmitBreadcrumbsSequential(PlayerBase player, int stepMs)
	{
	    if (!player) 
			return;
	
	    vector playerPos = player.GetPosition();
	    float range2 = m_ZenAnimalTrackMaxRangeMeters * m_ZenAnimalTrackMaxRangeMeters;
	
	    // Collect candidates within range (store both index and d^2)
	    array<int>   candIdx = new array<int>();
	    array<float> candD2  = new array<float>();
		
	    // Collect candidates within range
		for (int i = 0; i < m_ZenAnimalTrackBreadcrumbs.Count(); i++)
		{
		    vector p = m_ZenAnimalTrackBreadcrumbs[i];
		    float d2 = ZenSkills_DistanceSq(playerPos, p); // correct
		    if (d2 <= range2)
		    {
		        candIdx.Insert(i);
		        candD2.Insert(d2);
		    }
		}
		
	    if (candIdx.Count() == 0) 
			return;
	
	    // Pick K closest (selection)
	    int k = Math.Min(candIdx.Count(), ZEN_BURST_MAX_PLAY);
	    array<int>   chosen   = new array<int>();
	    array<float> chosenD2 = new array<float>();
		
	    chosen.Reserve(k); 
		chosenD2.Reserve(k);
	
	    for (int pick = 0; pick < k; pick++)
	    {
	        int best = -1;
	        float bestD2 = 1e30;
	        for (int t = 0; t < candIdx.Count(); t++)
	        {
	            float d2t = candD2[t];
	            if (d2t < bestD2)
	            {
	                bestD2 = d2t;
	                best = t;
	            }
	        }
			
	        if (best == -1) 
				break;
			
	        chosen.Insert(candIdx[best]);
	        chosenD2.Insert(candD2[best]);
	        candD2[best] = 1e30; // mark used
	    }
	
	    // Order the chosen by distance (closest -> farthest) so nearby ones play first
	    for (int i2 = 1; i2 < chosen.Count(); i2++)
	    {
	        int   idxMove  = chosen[i2];
	        float d2Move   = chosenD2[i2];
	        int   j        = i2 - 1;
			
	        while (j >= 0 && chosenD2[j] > d2Move)
	        {
	            chosen[j + 1]   = chosen[j];
	            chosenD2[j + 1] = chosenD2[j];
	            j--;
	        }
			
	        chosen[j + 1]   = idxMove;
	        chosenD2[j + 1] = d2Move;
	    }
	
	    // Schedule sequential plays (with current burst token)
	    int ticket = ++m_ZenAnimalTrackBurstToken;
	    for (int c = 0; c < chosen.Count(); c++)
	    {
	        int idx = chosen[c];
	        vector pos = m_ZenAnimalTrackBreadcrumbs[idx];
	        Param3<AnimalBase, vector, int> payload = new Param3<AnimalBase, vector, int>(this, pos, ticket);
	        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(AnimalBase.Zen_PlayTrailAt, c * stepMs, false, payload);
	    }
	}

    // Static callback for CallLater
    static void Zen_PlayTrailAt(Param3<AnimalBase, vector, int> data)
    {
        if (!data) 
			return;
		
        AnimalBase animal = data.param1;
        if (!animal) 
			return;
		
        int ticket = data.param3;
        if (ticket != animal.m_ZenAnimalTrackBurstToken) 
			return; // canceled/stale burst

        vector pos = data.param2;
        ParticleManager.GetInstance().PlayInWorld(animal.GetZenSkillsTrackerParticleType(pos), pos);
    }

    protected void CullExcessZenTrackerParticles()
    {
        int overflow = m_ZenAnimalTrackBreadcrumbs.Count() - ZEN_ANIMAL_TRACKING_MAX_CRUMBS;
        if (overflow <= 0) 
			return;
		
        while (overflow > 0 && m_ZenAnimalTrackBreadcrumbs.Count() > 0)
		{
			overflow--;
            m_ZenAnimalTrackBreadcrumbs.Remove(0); // FIFO
		}
    }

	protected ref map<string, int> m_ZenSurfacePidCache = new map<string, int>();

	int GetZenSkillsTrackerParticleType(vector worldPos)
	{
	    string surf;
	    GetGame().SurfaceGetType(worldPos[0], worldPos[2], surf);
	    surf.ToLower();
	
	    int pid;
	    if (m_ZenSurfacePidCache.Find(surf, pid))
	        return pid;
	
	    // Heuristics: adjust/add as you like
	    if (surf.Contains("snow"))       
			pid = ParticleList.IMPACT_SNOW_ENTER;
	    else if (surf.Contains("ice"))   
			pid = ParticleList.IMPACT_ICE_ENTER;
	    else if (surf.Contains("sand"))  
			pid = ParticleList.IMPACT_SAND_ENTER;
	    else if (surf.Contains("gravel"))
			pid = ParticleList.IMPACT_GRAVEL_ENTER;
	    else if (surf.Contains("asphalt") || surf.Contains("road"))  
			pid = ParticleList.IMPACT_CONCRETE_ENTER;
	    else if (surf.Contains("grass") || surf.Contains("meadow"))
			pid = ParticleList.IMPACT_GRASS_ENTER;
	    else if (surf.Contains("forest") || surf.Contains("soil") || surf.Contains("dirt") || surf.Contains("mud"))   
			pid = ParticleList.IMPACT_DIRT_ENTER;
	    else if (surf.Contains("foliage") || surf.Contains("leaf"))  
			pid = ParticleList.IMPACT_FOLIAGE_ENTER;
	    else                             
			pid = ParticleList.IMPACT_DISTANT_DUST;
	
	    m_ZenSurfacePidCache.Insert(surf, pid);
	    return pid;
	}

    protected void SpawnZenBreadcrumb(vector worldPos)
    {
        // Avoid duplicates if the same exact vector lands twice
        if (m_ZenAnimalTrackBreadcrumbs.Find(worldPos) == -1)
            m_ZenAnimalTrackBreadcrumbs.Insert(worldPos);
		
        CullExcessZenTrackerParticles();
    }
    #endif
	
	//! SHARED CODE
	static float ZenSkills_DistanceSq(vector a, vector b)
	{
	    vector d = a - b;
	    return d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
	}
	
	//override void EEKilled(Object killer)
	//{
		//super.EEKilled(killer);
		
		//ZenSkillFunctions.HandleEntityKilledEXP(this, killer);
	//}
	
	// For some reason, EEKilled() doesn't reliably register the killer for AnimalBase. 
	// Sometimes it will pass the animal as its own killer? Possible game engine bug?
	// This is the only method I could come up with which reliably works.
	protected bool m_ZenSkillsWasAlive = false;
	
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		m_ZenSkillsWasAlive = IsAlive();
		
		return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	}
	
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		
		if (!source)
			return;
		
		if (GetGame().IsDedicatedServer() && !IsAlive() && m_ZenSkillsWasAlive)
		{
			PlayerBase player = PlayerBase.Cast(source.GetHierarchyRootPlayer());
			if (!player)
				return;
			
			ZenSkillFunctions.HandleEntityKilledEXP(this, player);
			m_ZenSkillsWasAlive = false; // failsafe - should be set on EEOnDamageCalculated on dead bodies when hit, but can't hurt.
		}
	}
}
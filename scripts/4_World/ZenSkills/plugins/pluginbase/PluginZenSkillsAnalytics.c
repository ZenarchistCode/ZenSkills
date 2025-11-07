// ================================================================
//  PluginZenSkillsAnalytics (no ternary operators)
//  - Tracks average EXP gained per skill per PLAYER per session
//  - Consolidates into global aggregates at finalize
//  - Checkpoint-safe during session
//  - File storage: $profile:Zenarchist/Skills/Analytics/TotalSessionAnalytics.json
// ================================================================

class ZenAnalyticsMasterRow
{
	int   CumExp;                      // cumulative EXP across finalized sessions
	int   AvgExp;                      // CumExp / GlobalSessions (computed on finalize)

	int   CumPlayers;                  // sum of "distinct player counts" across sessions
	int   SessionsWithPlayers;         // number of sessions where this skill had at least one contributing player

	float WeightedAvgPerPlayer;        // CumExp / CumPlayers (computed on finalize; weighted by players)
	float SumOfSessionPlayerMeans;     // sum of (session_total_exp / session_player_count) over sessions (internal aggregator)
	float MeanOfSessionMeans;          // SumOfSessionPlayerMeans / SessionsWithPlayers (computed on finalize)
}

class ZenAnalyticsActionRow
{
	int   CumExp;                    // cumulative EXP across finalized sessions
	int   AvgExp;                    // CumExp / GlobalSessions (recomputed on finalize)

	int   CumPlayers;                // sum of distinct-player counts across sessions
	int   SessionsWithPlayers;       // sessions where this action had >= 1 contributing player

	float WeightedAvgPerPlayer;      // CumExp / CumPlayers (recomputed on finalize)
	float SumOfSessionPlayerMeans;   // sum over sessions of (sessionExp / sessionPlayers)
	float MeanOfSessionMeans;        // SumOfSessionPlayerMeans / SessionsWithPlayers
}


class ZenAnalyticsMasterDB
{
	ref map<string, ref ZenAnalyticsMasterRow> PerSkill;
	ref map<string, ref ZenAnalyticsActionRow> PerAction;
	
	int    Sessions;
	int    LastUpdateTick;

	string OpenSessionId;
	ref map<string, int>               OpenSessionApplied;          // per-skill
	ref map<string, ref array<string>> OpenSessionDistinctPlayers;  // per-skill

	ref map<string, int>               OpenSessionAppliedAction;          // per-action totals for this session
	ref map<string, ref array<string>> OpenSessionDistinctPlayersAction;  // per-action distinct players set

	void ZenAnalyticsMasterDB()
	{
		PerSkill = new map<string, ref ZenAnalyticsMasterRow>();
		OpenSessionApplied = new map<string, int>();
		OpenSessionDistinctPlayers = new map<string, ref array<string>>();

		PerAction = new map<string, ref ZenAnalyticsActionRow>();
		OpenSessionAppliedAction = new map<string, int>();
		OpenSessionDistinctPlayersAction = new map<string, ref array<string>>();
	}
}


class PluginZenSkillsAnalytics extends PluginBase
{
	// -------- in-memory per-current-session state --------
	protected ref map<string, int>               m_SessionTotals;            // per-skill
	protected ref map<string, ref array<string>> m_SessionDistinctPlayers;   // per-skill
	
	// NEW: per-action for current session (in-memory)
	protected ref map<string, int>               m_SessionTotalsByAction;            // actionKey -> total EXP this session
	protected ref map<string, ref array<string>> m_SessionDistinctPlayersByAction;   // actionKey -> distinct player UIDs
	
	protected string m_OpenSessionId;
	protected int    m_OpenSessionStartTick;

	// -------- master persistence --------
	protected ref ZenAnalyticsMasterDB m_Master;

	// =============================== LIFECYCLE ===============================
	override void OnInit()
	{
		#ifdef ZENSKILLSDEBUG
		Print("[PluginZenSkillsAnalytics] :: OnInit.");
		#endif

		if (!GetGame().IsDedicatedServer())
			return;

		m_SessionTotals = new map<string, int>();
		m_SessionDistinctPlayers = new map<string, ref array<string>>();
		m_SessionTotalsByAction = new map<string, int>();
		m_SessionDistinctPlayersByAction = new map<string, ref array<string>>();
		m_Master = new ZenAnalyticsMasterDB();

		EnsureDir();
		LoadMaster();
		SessionBeginOrResume();
	}

	// ============================== PUBLIC API ===============================
	// Preferred: supply playerUID and actionKey so we can compute per-action stats.
	void OnExpAdded(string skillKey, int exp, string playerUID, string actionKey)
	{
		if (!GetGame().IsDedicatedServer()) return;
		if (exp <= 0) return;
		if (skillKey == "") return;
	
		// Ensure there's an open session
		if (m_OpenSessionId == "") { SessionBeginOrResume(); }
	
		// ----- per-skill totals (existing) -----
		int currentSkillTotal = 0;
		bool hadSkillTotal = m_SessionTotals.Find(skillKey, currentSkillTotal);
		if (hadSkillTotal) m_SessionTotals.Set(skillKey, currentSkillTotal + exp);
		else m_SessionTotals.Set(skillKey, exp);
	
		if (playerUID != "")
		{
			array<string> skillPlayers = m_SessionDistinctPlayers.Get(skillKey);
			if (!skillPlayers) { skillPlayers = new array<string>(); m_SessionDistinctPlayers.Set(skillKey, skillPlayers); }
			if (!ArrayContainsString(skillPlayers, playerUID)) { skillPlayers.Insert(playerUID); }
		}
	
		// ----- per-action totals (NEW) -----
		if (actionKey != "")
		{
			int currentActionTotal = 0;
			bool hadActionTotal = m_SessionTotalsByAction.Find(actionKey, currentActionTotal);
			if (hadActionTotal) m_SessionTotalsByAction.Set(actionKey, currentActionTotal + exp);
			else m_SessionTotalsByAction.Set(actionKey, exp);
	
			if (playerUID != "")
			{
				array<string> actionPlayers = m_SessionDistinctPlayersByAction.Get(actionKey);
				if (!actionPlayers) { actionPlayers = new array<string>(); m_SessionDistinctPlayersByAction.Set(actionKey, actionPlayers); }
				if (!ArrayContainsString(actionPlayers, playerUID)) { actionPlayers.Insert(playerUID); }
			}
		}
	
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[Analytics] +EXP skill=" + skillKey + " +" + exp + " action=" + actionKey + " skillTotal=" + m_SessionTotals.Get(skillKey));
		#endif
	}

	// Call this in your periodic autosave/checkpoint.
	void Checkpoint()
	{
		if (!GetGame().IsDedicatedServer())
			return;

		ApplyDeltasToMaster(); // updates OpenSessionApplied and OpenSessionDistinctPlayers from in-memory
		SaveMaster();

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[Analytics] Checkpoint saved. appliedKeys=" + m_Master.OpenSessionApplied.Count());
		#endif
	}

	// Call when the LAST player logs out (end of an uptime session).
	// Folds open-session data into global aggregates.
	void FinalizeSession()
	{
		if (!GetGame().IsDedicatedServer())
			return;

		if (m_OpenSessionId == "")
		{
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[Analytics] FinalizeSession: no open session, nothing to do.");
			#endif
			return;
		}

		// Ensure master has the latest numbers from memory
		ApplyDeltasToMaster();

		// Ensure rows exist for all known skills so zeros are counted in averages
		EnsureAllSkillRows();
		
		float sessionMeanPerPlayer;

		// Fold this session into cum stats (per skill)
		for (int skillIndex = 0; skillIndex < m_Master.PerSkill.Count(); skillIndex++)
		{
			string skillKey = m_Master.PerSkill.GetKey(skillIndex);
			ZenAnalyticsMasterRow row = m_Master.PerSkill.GetElement(skillIndex);
			if (!row)
				continue;

			// Get session totals and distinct players for this skill
			int sessionTotalExp = 0;
			bool hasApplied = m_Master.OpenSessionApplied.Find(skillKey, sessionTotalExp);

			array<string> sessionPlayerList = m_Master.OpenSessionDistinctPlayers.Get(skillKey);
			int sessionPlayerCount = 0;
			if (sessionPlayerList)
			{
				sessionPlayerCount = sessionPlayerList.Count();
			}

			if (hasApplied)
			{
				row.CumExp = row.CumExp + sessionTotalExp;

				if (sessionPlayerCount > 0)
				{
					row.CumPlayers = row.CumPlayers + sessionPlayerCount;
					row.SessionsWithPlayers = row.SessionsWithPlayers + 1;

					sessionMeanPerPlayer = sessionTotalExp / (float)sessionPlayerCount;
					row.SumOfSessionPlayerMeans = row.SumOfSessionPlayerMeans + sessionMeanPerPlayer;
				}
			}

			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[Analytics] Finalize skill=" + skillKey + " sessionExp=" + sessionTotalExp + " players=" + sessionPlayerCount + " CumExp=" + row.CumExp);
			#endif
		}
		
		// Create rows for observed actions this session
		for (int i = 0; i < m_Master.OpenSessionAppliedAction.Count(); i++)
		{
			string actionKey = m_Master.OpenSessionAppliedAction.GetKey(i);
			EnsureActionRow(actionKey);
		}
		
		// Aggregate this session into per-action rows
		for (int j = 0; j < m_Master.PerAction.Count(); j++)
		{
			string actionKey2 = m_Master.PerAction.GetKey(j);
			ZenAnalyticsActionRow actionRow = m_Master.PerAction.GetElement(j);
			if (!actionRow) continue;
		
			int sessionActionExp = 0;
			bool hasAppliedAction = m_Master.OpenSessionAppliedAction.Find(actionKey2, sessionActionExp);
		
			array<string> actionPlayers = m_Master.OpenSessionDistinctPlayersAction.Get(actionKey2);
			int sessionActionPlayerCount = 0;
			if (actionPlayers) sessionActionPlayerCount = actionPlayers.Count();
		
			if (hasAppliedAction)
			{
				actionRow.CumExp = actionRow.CumExp + sessionActionExp;
		
				if (sessionActionPlayerCount > 0)
				{
					actionRow.CumPlayers = actionRow.CumPlayers + sessionActionPlayerCount;
					actionRow.SessionsWithPlayers = actionRow.SessionsWithPlayers + 1;
		
					sessionMeanPerPlayer = sessionActionExp / (float)sessionActionPlayerCount;
					actionRow.SumOfSessionPlayerMeans = actionRow.SumOfSessionPlayerMeans + sessionMeanPerPlayer;
				}
			}
		
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[Analytics] Finalize action=" + actionKey2 + " sessionExp=" + sessionActionExp + " players=" + sessionActionPlayerCount + " CumExp=" + actionRow.CumExp);
			#endif
		}

		// Bump global session count once
		m_Master.Sessions = m_Master.Sessions + 1;

		// Recompute derived metrics for all skills
		for (int recomputeIndex = 0; recomputeIndex < m_Master.PerSkill.Count(); recomputeIndex++)
		{
			ZenAnalyticsMasterRow recomputeRow = m_Master.PerSkill.GetElement(recomputeIndex);
			if (!recomputeRow)
				continue;

			// AvgExp per session (classic)
			if (m_Master.Sessions > 0)
			{
				recomputeRow.AvgExp = recomputeRow.CumExp / m_Master.Sessions;
			}
			else
			{
				recomputeRow.AvgExp = 0;
			}

			// WeightedAvgPerPlayer across all sessions
			if (recomputeRow.CumPlayers > 0)
			{
				recomputeRow.WeightedAvgPerPlayer = recomputeRow.CumExp / (float)recomputeRow.CumPlayers;
			}
			else
			{
				recomputeRow.WeightedAvgPerPlayer = 0.0;
			}

			// Mean of session means (unweighted by session size)
			if (recomputeRow.SessionsWithPlayers > 0)
			{
				recomputeRow.MeanOfSessionMeans = recomputeRow.SumOfSessionPlayerMeans / (float)recomputeRow.SessionsWithPlayers;
			}
			else
			{
				recomputeRow.MeanOfSessionMeans = 0.0;
			}
		}
		
		// Recompute derived metrics for actions
		for (int k = 0; k < m_Master.PerAction.Count(); k++)
		{
			ZenAnalyticsActionRow recompute = m_Master.PerAction.GetElement(k);
			if (!recompute) continue;
		
			// AvgExp per session
			if (m_Master.Sessions > 0) recompute.AvgExp = recompute.CumExp / m_Master.Sessions;
			else recompute.AvgExp = 0;
		
			// Weighted average per player
			if (recompute.CumPlayers > 0) recompute.WeightedAvgPerPlayer = recompute.CumExp / (float)recompute.CumPlayers;
			else recompute.WeightedAvgPerPlayer = 0.0;
		
			// Mean of session means (only sessions with contributors)
			if (recompute.SessionsWithPlayers > 0) recompute.MeanOfSessionMeans = recompute.SumOfSessionPlayerMeans / (float)recompute.SessionsWithPlayers;
			else recompute.MeanOfSessionMeans = 0.0;
		}
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[Analytics] FINALIZED session id=" + m_OpenSessionId + " sessions=" + m_Master.Sessions);
		#endif

		// Close session
		m_Master.OpenSessionId = "";
		m_Master.OpenSessionApplied.Clear();
		m_Master.OpenSessionDistinctPlayers.Clear();

		m_OpenSessionId = "";
		m_SessionTotals.Clear();
		m_SessionDistinctPlayers.Clear();
		
		m_Master.OpenSessionAppliedAction.Clear();
		m_Master.OpenSessionDistinctPlayersAction.Clear();
		
		m_SessionTotalsByAction.Clear();
		m_SessionDistinctPlayersByAction.Clear();

		SaveMaster();
	}

	// ---------------- Convenience getters (admin UI / debugging) ----------------
	int GetAverageExpPerSession(string skillKey)
	{
		if (!GetGame().IsDedicatedServer())
			return 0;

		LoadMaster();
		ZenAnalyticsMasterRow row = m_Master.PerSkill.Get(skillKey);
		if (row)
		{
			return row.AvgExp;
		}
		return 0;
	}

	float GetWeightedAvgPerPlayer(string skillKey)
	{
		if (!GetGame().IsDedicatedServer())
			return 0.0;

		LoadMaster();
		ZenAnalyticsMasterRow row = m_Master.PerSkill.Get(skillKey);
		if (row)
		{
			return row.WeightedAvgPerPlayer;
		}
		return 0.0;
	}

	float GetMeanOfSessionMeansPerPlayer(string skillKey)
	{
		if (!GetGame().IsDedicatedServer())
			return 0.0;

		LoadMaster();
		ZenAnalyticsMasterRow row = m_Master.PerSkill.Get(skillKey);
		if (row)
		{
			return row.MeanOfSessionMeans;
		}
		return 0.0;
	}

	int GetSessionsCount()
	{
		if (!GetGame().IsDedicatedServer())
			return 0;

		LoadMaster();
		return m_Master.Sessions;
	}
	
	int GetAverageExpPerSessionForAction(string actionKey)
	{
		if (!GetGame().IsDedicatedServer()) return 0;
		LoadMaster();
		ZenAnalyticsActionRow row = m_Master.PerAction.Get(actionKey);
		if (row) return row.AvgExp;
		return 0;
	}
	
	float GetWeightedAvgPerPlayerForAction(string actionKey)
	{
		if (!GetGame().IsDedicatedServer()) return 0.0;
		LoadMaster();
		ZenAnalyticsActionRow row = m_Master.PerAction.Get(actionKey);
		if (row) return row.WeightedAvgPerPlayer;
		return 0.0;
	}
	
	float GetMeanOfSessionMeansPerPlayerForAction(string actionKey)
	{
		if (!GetGame().IsDedicatedServer()) return 0.0;
		LoadMaster();
		ZenAnalyticsActionRow row = m_Master.PerAction.Get(actionKey);
		if (row) return row.MeanOfSessionMeans;
		return 0.0;
	}

	// ============================== INTERNALS ===============================
	protected string DirPath()
	{
		return "$profile:Zenarchist/Skills/Analytics";
	}

	protected string MasterPath()
	{
		return DirPath() + "/TotalSessionAnalytics.json";
	}

	protected void EnsureDir()
	{
		string directoryPath = DirPath();
		if (!FileExist(directoryPath))
		{
			MakeDirectory(directoryPath);

			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[Analytics] Created dir: " + directoryPath);
			#endif
		}
	}

	protected void LoadMaster()
	{
		string path = MasterPath();
		if (FileExist(path))
		{
			ZenAnalyticsMasterDB loaded = new ZenAnalyticsMasterDB();
			JsonFileLoader<ZenAnalyticsMasterDB>.JsonLoadFile(path, loaded);

			if (loaded)
			{
				if (loaded.PerSkill) m_Master.PerSkill = loaded.PerSkill;
				if (loaded.OpenSessionApplied) m_Master.OpenSessionApplied = loaded.OpenSessionApplied;
				if (loaded.OpenSessionDistinctPlayers) m_Master.OpenSessionDistinctPlayers = loaded.OpenSessionDistinctPlayers;
			
				// NEW:
				if (loaded.PerAction) m_Master.PerAction = loaded.PerAction;
				if (loaded.OpenSessionAppliedAction) m_Master.OpenSessionAppliedAction = loaded.OpenSessionAppliedAction;
				if (loaded.OpenSessionDistinctPlayersAction) m_Master.OpenSessionDistinctPlayersAction = loaded.OpenSessionDistinctPlayersAction;
			
				m_Master.Sessions = loaded.Sessions;
				m_Master.LastUpdateTick = loaded.LastUpdateTick;
				m_Master.OpenSessionId = loaded.OpenSessionId;
			}
		}

		// Ensure non-null:
		if (!m_Master.PerSkill) m_Master.PerSkill = new map<string, ref ZenAnalyticsMasterRow>();
		if (!m_Master.OpenSessionApplied) m_Master.OpenSessionApplied = new map<string, int>();
		if (!m_Master.OpenSessionDistinctPlayers) m_Master.OpenSessionDistinctPlayers = new map<string, ref array<string>>();
		
		if (!m_Master.PerAction) m_Master.PerAction = new map<string, ref ZenAnalyticsActionRow>();
		if (!m_Master.OpenSessionAppliedAction) m_Master.OpenSessionAppliedAction = new map<string, int>();
		if (!m_Master.OpenSessionDistinctPlayersAction) m_Master.OpenSessionDistinctPlayersAction = new map<string, ref array<string>>();
	}

	protected void SaveMaster()
	{
		m_Master.LastUpdateTick = GetGame().GetTime();
		JsonFileLoader<ZenAnalyticsMasterDB>.JsonSaveFile(MasterPath(), m_Master);
	}

	protected void SessionBeginOrResume()
	{
		// Resume if master says there is an open session
		if (m_Master.OpenSessionId != "")
		{
			m_OpenSessionId = m_Master.OpenSessionId;
			
			// Prime in-memory per-skill totals from applied
			m_SessionTotals.Clear();
			for (int idxSkill = 0; idxSkill < m_Master.OpenSessionApplied.Count(); idxSkill++)
			{
			    string skillKeyResume = m_Master.OpenSessionApplied.GetKey(idxSkill);
			    int appliedValueSkill = m_Master.OpenSessionApplied.GetElement(idxSkill);
			    m_SessionTotals.Set(skillKeyResume, appliedValueSkill);
			}
			
			// Prime in-memory per-skill distinct players
			m_SessionDistinctPlayers.Clear();
			for (int idxSkillPlayers = 0; idxSkillPlayers < m_Master.OpenSessionDistinctPlayers.Count(); idxSkillPlayers++)
			{
			    string skillKeyPlayers = m_Master.OpenSessionDistinctPlayers.GetKey(idxSkillPlayers);
			    array<string> persistedPlayersSkill = m_Master.OpenSessionDistinctPlayers.GetElement(idxSkillPlayers);
			
			    array<string> cloneSkillPlayers = new array<string>();
			    if (persistedPlayersSkill)
			    {
			        for (int copyIndex = 0; copyIndex < persistedPlayersSkill.Count(); copyIndex++)
			        {
			            cloneSkillPlayers.Insert(persistedPlayersSkill.Get(copyIndex));
			        }
			    }
			    m_SessionDistinctPlayers.Set(skillKeyPlayers, cloneSkillPlayers);
			}

			// Prime in-memory per-action totals from applied
			m_SessionTotalsByAction.Clear();
			for (int i = 0; i < m_Master.OpenSessionAppliedAction.Count(); i++)
			{
				string actionKey = m_Master.OpenSessionAppliedAction.GetKey(i);
				int appliedValue = m_Master.OpenSessionAppliedAction.GetElement(i);
				m_SessionTotalsByAction.Set(actionKey, appliedValue);
			}
			
			// Prime in-memory per-action distinct players
			m_SessionDistinctPlayersByAction.Clear();
			for (int j = 0; j < m_Master.OpenSessionDistinctPlayersAction.Count(); j++)
			{
				string actionKey2 = m_Master.OpenSessionDistinctPlayersAction.GetKey(j);
				array<string> persistedPlayers = m_Master.OpenSessionDistinctPlayersAction.GetElement(j);
				array<string> clone = new array<string>();
				if (persistedPlayers)
				{
					for (int cp = 0; cp < persistedPlayers.Count(); cp++) { clone.Insert(persistedPlayers.Get(cp)); }
				}
				m_SessionDistinctPlayersByAction.Set(actionKey2, clone);
			}

			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[Analytics] Resumed open session id=" + m_OpenSessionId + " keys=" + m_SessionTotals.Count());
			#endif

			return;
		}

		// Start a new session id
		int year;
		int month;
		int day;
		int hour;
		int minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		int tick = GetGame().GetTime();

		string sy = year.ToStringLen(4);
		string sm = month.ToStringLen(2);
		string sd = day.ToStringLen(2);
		string sh = hour.ToStringLen(2);
		string smin = minute.ToStringLen(2);

		m_OpenSessionId = sy + "-" + sm + "-" + sd + "_" + sh + "-" + smin + "_" + tick.ToString();

		m_OpenSessionStartTick = tick;
		m_SessionTotals.Clear();
		m_SessionDistinctPlayers.Clear();

		m_Master.OpenSessionId = m_OpenSessionId;
		m_Master.OpenSessionApplied.Clear();
		m_Master.OpenSessionDistinctPlayers.Clear();
		
		m_SessionTotalsByAction.Clear();
		m_SessionDistinctPlayersByAction.Clear();
		
		m_Master.OpenSessionAppliedAction.Clear();
		m_Master.OpenSessionDistinctPlayersAction.Clear();
		
		SaveMaster();

		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[Analytics] New session id=" + m_OpenSessionId);
		#endif
	}

	// Push in-memory deltas into master (no finalize)
	protected void ApplyDeltasToMaster()
	{
		if (m_OpenSessionId == "")
		{
			SessionBeginOrResume();
		}
		
		int a, i;
		int currentTotal;
		int appliedValue;
		bool hasApplied;
		string uid;
		array<string> memPlayers;
		array<string> persisted;

		// Ensure master aligned to this open session id
		if (m_Master.OpenSessionId != m_OpenSessionId)
		{
			m_Master.OpenSessionId = m_OpenSessionId;
			m_Master.OpenSessionApplied.Clear();
			m_Master.OpenSessionDistinctPlayers.Clear();
		
			// NEW: reset action mirrors when session id changes
			m_Master.OpenSessionAppliedAction.Clear();
			m_Master.OpenSessionDistinctPlayersAction.Clear();
		}

		// Update applied totals to match in-memory
		for (i = 0; i < m_SessionTotals.Count(); i++)
		{
			string skillKey = m_SessionTotals.GetKey(i);
			currentTotal = m_SessionTotals.GetElement(i);

			appliedValue = 0;
			hasApplied = m_Master.OpenSessionApplied.Find(skillKey, appliedValue);
			if (!hasApplied || currentTotal > appliedValue)
			{
				m_Master.OpenSessionApplied.Set(skillKey, currentTotal);
				#ifdef ZENSKILLSDEBUG
				ZenSkillsPrint("[Analytics] Apply delta skill=" + skillKey + " applied->" + currentTotal);
				#endif
			}
		}

		// Update distinct player sets to match in-memory
		for (int j = 0; j < m_SessionDistinctPlayers.Count(); j++)
		{
			string skillKey2 = m_SessionDistinctPlayers.GetKey(j);
			memPlayers = m_SessionDistinctPlayers.GetElement(j);
			if (!memPlayers)
				continue;

			persisted = m_Master.OpenSessionDistinctPlayers.Get(skillKey2);
			if (!persisted)
			{
				persisted = new array<string>();
				m_Master.OpenSessionDistinctPlayers.Set(skillKey2, persisted);
			}

			// Insert any missing players
			for (a = 0; a < memPlayers.Count(); a++)
			{
				uid = memPlayers.Get(a);
				if (!ArrayContainsString(persisted, uid))
				{
					persisted.Insert(uid);
				}
			}
		}
		
		for (a = 0; a < m_SessionTotalsByAction.Count(); a++)
		{
			string actionKey = m_SessionTotalsByAction.GetKey(a);
			currentTotal = m_SessionTotalsByAction.GetElement(a);
		
			appliedValue = 0;
			hasApplied = m_Master.OpenSessionAppliedAction.Find(actionKey, appliedValue);
			if (!hasApplied || currentTotal > appliedValue)
			{
				m_Master.OpenSessionAppliedAction.Set(actionKey, currentTotal);
				#ifdef ZENSKILLSDEBUG
				ZenSkillsPrint("[Analytics] Apply delta action=" + actionKey + " applied->" + currentTotal);
				#endif
			}
		}
		
		for (int b = 0; b < m_SessionDistinctPlayersByAction.Count(); b++)
		{
			string actionKey2 = m_SessionDistinctPlayersByAction.GetKey(b);
			memPlayers = m_SessionDistinctPlayersByAction.GetElement(b);
			if (!memPlayers) continue;
		
			persisted = m_Master.OpenSessionDistinctPlayersAction.Get(actionKey2);
			if (!persisted)
			{
				persisted = new array<string>();
				m_Master.OpenSessionDistinctPlayersAction.Set(actionKey2, persisted);
			}
		
			for (i = 0; i < memPlayers.Count(); i++)
			{
				uid = memPlayers.Get(i);
				if (!ArrayContainsString(persisted, uid))
				{
					persisted.Insert(uid);
				}
			}
		}
	}

	// Ensure a row exists for a skill in the master map
	protected ZenAnalyticsMasterRow EnsureRow(string skillKey)
	{
		ZenAnalyticsMasterRow row = m_Master.PerSkill.Get(skillKey);
		if (!row)
		{
			row = new ZenAnalyticsMasterRow();
			row.CumExp = 0;
			row.AvgExp = 0;
			row.CumPlayers = 0;
			row.SessionsWithPlayers = 0;
			row.WeightedAvgPerPlayer = 0.0;
			row.SumOfSessionPlayerMeans = 0.0;
			row.MeanOfSessionMeans = 0.0;

			m_Master.PerSkill.Set(skillKey, row);
		}
		return row;
	}

	// Make sure every defined skill has a row (so zeros get averaged properly)
	protected void EnsureAllSkillRows()
	{
		if (!GetZenSkillsConfig())
			return;
		if (!GetZenSkillsConfig().SkillDefs)
			return;

		for (int i = 0; i < GetZenSkillsConfig().SkillDefs.Count(); i++)
		{
			string skillKey = GetZenSkillsConfig().SkillDefs.GetKey(i);
			EnsureRow(skillKey);
		}
	}

	// Utility: simple membership check for array<string>
	protected bool ArrayContainsString(array<string> arr, string value)
	{
		if (!arr)
			return false;

		for (int i = 0; i < arr.Count(); i++)
		{
			if (arr.Get(i) == value)
			{
				return true;
			}
		}
		return false;
	}
	
	protected ZenAnalyticsActionRow EnsureActionRow(string actionKey)
	{
		ZenAnalyticsActionRow row = m_Master.PerAction.Get(actionKey);
		if (!row)
		{
			row = new ZenAnalyticsActionRow();
			row.CumExp = 0;
			row.AvgExp = 0;
			row.CumPlayers = 0;
			row.SessionsWithPlayers = 0;
			row.WeightedAvgPerPlayer = 0.0;
			row.SumOfSessionPlayerMeans = 0.0;
			row.MeanOfSessionMeans = 0.0;
			m_Master.PerAction.Set(actionKey, row);
		}
		return row;
	}
}

// =================== Global accessor (unchanged name) ===================
static ref PluginZenSkillsAnalytics m_PluginZenSkillsAnalytics;

static PluginZenSkillsAnalytics GetZenSkillsAnalyticsPlugin()
{
	if (!m_PluginZenSkillsAnalytics)
	{
		m_PluginZenSkillsAnalytics = PluginZenSkillsAnalytics.Cast(GetPlugin(PluginZenSkillsAnalytics));
	}
	return m_PluginZenSkillsAnalytics;
}

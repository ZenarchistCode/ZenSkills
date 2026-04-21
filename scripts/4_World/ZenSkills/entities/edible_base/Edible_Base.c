modded class Edible_Base
{
	void SetZenSkillsNoSalmonella(bool b);
	
	protected bool m_ZenSkillsTriggerDust;
	protected bool m_ZenSkillsRewardedEXP;
	protected PlayerBase m_ZenSkillsLastPlayerChef;
	
	void TriggerZenParticleDust()
	{
		// Delay particle send because items spawn @ 0 0 0 coords.
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TriggerZenParticleDustSync, 30);
	}
	
	void TriggerZenParticleDustSync()
	{		
		m_ZenSkillsTriggerDust = true;
		SetSynchDirty();
	}
	
	void Edible_Base()
	{
		RegisterNetSyncVariableBoolSignal("m_ZenSkillsTriggerDust");
	}
	
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		
		if (m_ZenSkillsTriggerDust)
		{
			ParticleManager.GetInstance().PlayInWorld(ParticleList.IMPACT_DISTANT_DUST, GetPosition());
		}
	}
	
	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);
		
		HandleZenSkillsCooking(oldLoc, newLoc);
	}
	
	void HandleZenSkillsCooking(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		PlayerBase mover = PlayerBase.Cast(oldLoc.GetParent()); // the player moving it into cookware
		if (!mover) 
		{
			mover = PlayerBase.Cast(GetHierarchyRootPlayer()); // fallback (hands > attach)
		}
		
		if (!mover)
		{
			mover = PlayerBase.Cast(newLoc.GetParent());
		}
		
		if (!mover || !mover.GetIdentity())
		{
			return;
		}
		
		m_ZenSkillsLastPlayerChef = mover;
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkills] Chef " + m_ZenSkillsLastPlayerChef.GetIdentity().GetId() + " assigned to food " + GetType());
		#endif
	}
	
	override void OnFoodStageChange(FoodStageType stageOld, FoodStageType stageNew)
	{
		super.OnFoodStageChange(stageOld, stageNew);
		
		HandleZenSkillsFoodChange(stageOld, stageNew);
	}
	
	void HandleZenSkillsFoodChange(FoodStageType stageOld, FoodStageType stageNew)
	{
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkills] Food " + GetType() + " :: HandleZenSkillsFoodChange :: stageNew=" + stageNew);
		#endif
		
		if (!m_ZenSkillsLastPlayerChef || !m_ZenSkillsLastPlayerChef.GetIdentity() || !g_Game.IsDedicatedServer() || m_ZenSkillsRewardedEXP) 
			return;

		bool becameEdible = (stageOld == FoodStageType.RAW) && (stageNew == FoodStageType.BAKED || stageNew == FoodStageType.BOILED || stageNew == FoodStageType.DRIED);
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkills] Food " + GetType() + " was cooked= " + becameEdible);
		#endif
		
		if (!becameEdible) 
			return;
		
		if (m_ZenSkillsLastPlayerChef)
		{
			GetZenSkillsPlugin().AddEXP_Action(m_ZenSkillsLastPlayerChef, "Cooking");
			m_ZenSkillsRewardedEXP = true;
			
			#ifdef ZENSKILLSDEBUG
			ZenSkillsPrint("[ZenSkills] Chef " + m_ZenSkillsLastPlayerChef.GetIdentity().GetId() + " awarded EXP for cooking food " + GetType());
			#endif
		}
	}
	
	// Ensures player is awarded EXP if they login with food item and drag it onto fireplace slot directly without ever manipulating it first
	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		
		PlayerBase pb = PlayerBase.Cast(GetHierarchyRootPlayer());
		if (!pb || !pb.GetIdentity())
			return;
		
		m_ZenSkillsLastPlayerChef = pb;
		
		#ifdef ZENSKILLSDEBUG
		ZenSkillsPrint("[ZenSkills] Chef " + m_ZenSkillsLastPlayerChef.GetIdentity().GetId() + " assigned to food on cargo spawn " + GetType());
		#endif
	}
}
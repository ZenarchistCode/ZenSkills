modded class CarchingResultFishingAction : CatchingResultBase
{
	override EntityAI SpawnAndSetup(out int yItemIdx, vector v = vector.Zero)
	{
		EntityAI catch = super.SpawnAndSetup(yItemIdx, v);
		
		if (catch != null && m_Owner != null && GetGame().IsDedicatedServer())
		{
			Edible_Base food = Edible_Base.Cast(catch);
			if (food)
			{
				PlayerBase fisherman = PlayerBase.Cast(m_Owner.GetHierarchyRootPlayer());
				if (fisherman)
				{
					GetZenSkillsPlugin().AddEXP_Action(fisherman, "CaughtFish", food.GetQuantityNormalized());
				}
			}
		}

		return catch;
	}
}
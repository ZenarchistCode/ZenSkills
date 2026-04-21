modded class ZombieBase 
{
	// ZenModCore has a more robust method of detecting killer entity.
	override void EEKilledZen(notnull Object killer)
	{
		super.EEKilledZen(killer);
		
		ZenSkillFunctions.HandleEntityKilledEXP(this, killer);
	}
}
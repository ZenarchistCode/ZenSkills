modded class ZombieBase 
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		ZenSkillFunctions.HandleEntityKilledEXP(this, killer);
	}
}
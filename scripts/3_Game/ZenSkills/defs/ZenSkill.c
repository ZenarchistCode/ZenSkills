class ZenSkillDef
{
    string DisplayName;						// Display name in-game
    string Description;						// Description text
	int StartingEXP;						// Universal starting EXP for new players
    int EXP_Per_Perk;						// EXP cost to unlock a perk
    float EXP_Perk_Modifier;				// EXP nerf (or boost if > 1) based on how many perks are unlocked (makes levelling slower over time)
	float EXP_Refund_Modifier;				// How much EXP is lost per activated perk when reset from the in-game menu (NOT the perk injector)
    int MaxAllowedPerks;					// Max number of activated perks at any one time (set to 30 to enable all perks to be unlocked)
    ref map<string, ref ZenPerkDef> Perks;	// List of perk definitions for each tier (1_1, 1_2, 1_3, 2_1, 2_2, 3_1, 3_2, 3_3, 4_1, 4_2)

    void ZenSkillDef(string p_displayName = "", string p_description = "")
    {
		DisplayName = p_displayName;
		Description = p_description;
		StartingEXP = 0;
        EXP_Per_Perk = 1000;
        EXP_Perk_Modifier = 0.01;
        MaxAllowedPerks = 20;
		EXP_Refund_Modifier = 0.5;
        Perks = new map<string, ref ZenPerkDef>();
    }
	
	void CopySkillDef(ZenSkillDef def)
	{
		DisplayName = def.DisplayName;
		Description = def.Description;
		StartingEXP = def.StartingEXP;
        EXP_Per_Perk = def.EXP_Per_Perk;
        EXP_Perk_Modifier = def.EXP_Perk_Modifier;
        MaxAllowedPerks = def.MaxAllowedPerks;
		EXP_Refund_Modifier = def.EXP_Refund_Modifier;
		
		if (!Perks)
			Perks = new map<string, ref ZenPerkDef>();
		
		Perks.Clear();
		
		foreach (string key, ZenPerkDef perkDef : def.Perks)
		{
			Perks.Set(key, perkDef);
		}
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("\n\nCopySkillDef:" + ZenDebugToString());
		#endif
	}
	
	string ZenDebugToString()
	{
		string r = "";
		r = r + "\nDisplayName=" + DisplayName;
		r = r + "\nDescription=" + Description;
		r = r + "\nStartingEXP=" + StartingEXP;
		r = r + "\nEXP_Per_Perk=" + EXP_Per_Perk;
		r = r + "\nEXP_Perk_Modifier=" + EXP_Perk_Modifier;
		r = r + "\nEXP_Refund_Modifier=" + EXP_Refund_Modifier;
		r = r + "\nMaxAllowedPerks=" + MaxAllowedPerks;
		r = r + "\nPerksCount=" + Perks.Count();
		return r;
	}
}

class ZenSkill
{
	int EXP;
	ref map<string, ref ZenPerk> Perks;
	
	[NonSerialized()]
	ref ZenSkillDef SkillDef;
	
	void ZenSkill(int p_exp = 0)
	{
		EXP = p_exp;
		Perks = new map<string, ref ZenPerk>();
	}
	
	int CountPerksUnlocked()
	{
		int perkCount = 0;
		
		foreach (string key, ZenPerk perk : Perks)
		{
			perkCount += perk.Level;
		}
		
		return perkCount;
	}
	
	int CountPerksUnused()
	{
		return Math.Floor(EXP / GetDef().EXP_Per_Perk);
	}
	
	float ProgressToNextPerk()
	{
		float progress = (EXP / GetDef().EXP_Per_Perk) * 100;
		if (CountPerksUnused() > 0 || CountPerksUnlocked() >= GetDef().MaxAllowedPerks)
			progress = 100;
		
		return progress;
	}
	
	void SetDef(ZenSkillDef def)
	{
		SkillDef = def;
	}
	
	// Get skill config definition
	ZenSkillDef GetDef()
	{
		return SkillDef;
	}
}
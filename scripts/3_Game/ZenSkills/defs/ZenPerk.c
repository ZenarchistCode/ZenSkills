class ZenPerkDef
{
	string DisplayName;					// Display name in-game
	string Description;					// Description text
	string RewardSymbol;				// Relevant reward symbol for reward values
	int MinPerksRequiredFromPrevTier;	// Minimum perks required from lower tier before this tier can be unlocked
	ref array<float> Rewards;			// Reward value definitions
	
	void ZenPerkDef(string p_displayName = "", string p_description = "", string p_rewardSymbol = "", float p_reward1 = 5.0, float p_reward2 = 10.0, float p_reward3 = 15.0)
	{
		DisplayName = p_displayName;
		Description = p_description;
		RewardSymbol = p_rewardSymbol;
		MinPerksRequiredFromPrevTier = 1;
		Rewards = new array<float>();
		Rewards.Insert(p_reward1);
		Rewards.Insert(p_reward2);
		Rewards.Insert(p_reward3);
	}
	
	void CopyPerkDef(notnull ZenPerkDef def)
	{
		DisplayName = def.DisplayName;
		Description = def.Description;
		RewardSymbol = def.RewardSymbol;
		MinPerksRequiredFromPrevTier = def.MinPerksRequiredFromPrevTier;
		
		if (!Rewards)
			Rewards = new array<float>();
		
		Rewards.Clear();
		
		foreach (float reward : def.Rewards)
		{
			Rewards.Insert(reward);
		}
		
		#ifdef ZENSKILLSDEBUG 
		ZenSkillsPrint("\n\nCopyPerkDef:" + ZenDebugToString());
		#endif
	}
	
	string ZenDebugToString()
	{
		string r = "";
		r = r + "\nDisplayName=" + DisplayName;
		r = r + "\nDescription=" + Description;
		r = r + "\nRewardSymbol=" + RewardSymbol;
		r = r + "\nMinPerksRequiredFromPrevTier=" + MinPerksRequiredFromPrevTier;
		r = r + "\nRewards=" + Rewards.Count();
		
		for (int i = 0; i < Rewards.Count(); i++)
		{
			r = r + "\n  Reward[" + i + "]=" + Rewards.Get(i);
		}
		
		return r;
	}
}

class ZenPerk
{
	int Level;
	
	[NonSerialized()]
	ref ZenPerkDef PerkDef;
	
	void ZenPerk(int p_level = 0)
	{
		Level = p_level;
	}
	
	void SetDef(ZenPerkDef def)
	{
		PerkDef = def;
	}
	
	// Get skill perk config definition
	ZenPerkDef GetDef()
	{
		return PerkDef;
	}
	
	int GetRewardIndex()
	{
		return Level - 1;
	}
	
	float GetReward()
	{
		int rewardIndex = GetRewardIndex();
		if (rewardIndex >= 0 && rewardIndex < GetDef().Rewards.Count())
		{
			return GetDef().Rewards.Get(rewardIndex);
		}
		
		return 0;
	}
	
	float GetRewardPercent()
	{
		int reward = GetReward();
		
		if (reward > 0)
		{
			return reward;
		}
		
		return reward;
	}
	
	float GetRewardPercent01()
	{
		return Math.Clamp(GetRewardPercent() / 100, 0, 1);
	}
}
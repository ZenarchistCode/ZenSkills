class ZenSkillConstants
{
    static const int SKILL_GUI = 330101;
	static const int SKILL_HIGHSCORES = 330102;
	static const int SKILL_TUTORIAL = 330103;
	
	static const string KEY_INPUT_OPEN_SKILLS_GUI			= "UAZenSkillsOpen";

	static const string RPC = "ZenSkills_RPC";
	
	static const string RPC_ServerReceive_PerkUnlock 		= "RPC_ServerReceive_PerkUnlock";
	static const string RPC_ServerReceive_PerkReset 		= "RPC_ServerReceive_PerkReset";
	static const string RPC_ServerReceive_HighscoreRequest	= "RPC_ServerReceive_HighscoreRequest";
	
	static const string RPC_ClientReceive_ZenSkillsInit 	= "RPC_ClientReceive_ZenSkillsInit";
	static const string RPC_ClientReceive_PerkUpdate 		= "RPC_ClientReceive_PerkUpdate";
	static const string RPC_ClientReceive_ExpUpdate 		= "RPC_ClientReceive_ExpUpdate";
	static const string RPC_ClientReceive_PerkReset 		= "RPC_ClientReceive_PerkReset";
	static const string RPC_ClientReceive_Highscores		= "RPC_ClientReceive_Highscores";
	
	static const int RPC_ZenExpBoostNotify					= -87884201;
	
	static const int SOUND_PERKUNLOCKED						= 1;
	static const int SOUND_PERKUNLOCKED_FINAL				= 2;
	static const int SOUND_PERKSRESET						= 3;
	
	static const float ZEN_BUILDING_PERK_REFUND_AMT			= 0.5;
}

static bool ZEN_SKILLS_DEBUG_ON = false;

static void ZenSkillsPrint(string msg)
{
	if (!ZEN_SKILLS_DEBUG_ON)
		return;

	#ifdef SERVER
	string prefix = "[ZenSkills::SERVER] ";
	#else
	string prefix = "[ZenSkills::CLIENT] ";
	#endif
	
	Print(prefix + msg);
}
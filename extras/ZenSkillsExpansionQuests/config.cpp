/*
	(c) 2025 | ZenSkillsExpansionQuests | Zenarchist
*/

class CfgPatches
{
	class ZenSkillsExpansionQuests
	{
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"ZenSkills",
			"DayZExpansion_Quests_Scripts"
		};
	};
};

class CfgMods
{
	class ZenSkillsExpansionQuests
	{
		author = "Zenarchist";
		type = "mod";
		class defs
		{
			class engineScriptModule
			{
				files[]=
				{
					"ZenSkillsExpansionQuests/Scripts/1_Core"
				};
			};
			class gameLibScriptModule
			{
				files[]=
				{
					"ZenSkillsExpansionQuests/Scripts/2_GameLib"
				};
			};
			class gameScriptModule
			{
				value = "";
				files[] = 
				{ 
					"ZenSkillsExpansionQuests/Scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value = "";
				files[] = 
				{ 
					"ZenSkillsExpansionQuests/Scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value = "";
				files[] = 
				{ 
					"ZenSkillsExpansionQuests/Scripts/5_Mission"
				};
			};
		};
	};
};
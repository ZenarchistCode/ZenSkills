/*
	(c) 2025 | MapLinkZenSkillsCompatibility | Zenarchist
*/
class CfgPatches
{
	class MapLinkZenSkillsCompatibility
	{
		requiredAddons[] =
		{
			"ZenSkills",
			"MapLink"
		};
	};
};

class CfgMods
{
	class MapLinkZenSkillsCompatibility
	{
		author = "Zenarchist";
		type = "mod";
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = 
				{ 
					"MapLinkZenSkillsCompatibility/Scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value = "";
				files[] = 
				{ 
					"MapLinkZenSkillsCompatibility/Scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value = "";
				files[] = 
				{ 
					"MapLinkZenSkillsCompatibility/Scripts/5_Mission"
				};
			};
		};
	};
};
/*
	(c) 2025 | ZenSkills | Zenarchist
*/

class CfgPatches
{
	class ZenSkills
	{
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",

			"JM_CF_Scripts"
		};
	};
};

class CfgMods
{
	class ZenSkills
	{
		author = "Zenarchist";
		type = "mod";
		inputs = "ZenSkills/data/inputs.xml";
		class defs
		{
			class imageSets
			{
				files[] =
				{
					"ZenSkills/data/gui/imagesets/zen_imageset.imageset"
				};
			};
			class engineScriptModule
			{
				files[]=
				{
					"ZenSkills/Scripts/1_Core"
				};
			};
			class gameLibScriptModule
			{
				files[]=
				{
					"ZenSkills/Scripts/2_GameLib"
				};
			};
			class gameScriptModule
			{
				value = "";
				files[] = 
				{ 
					"ZenSkills/Scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value = "";
				files[] = 
				{ 
					"ZenSkills/Scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value = "";
				files[] = 
				{ 
					"ZenSkills/Scripts/5_Mission"
				};
			};
		};
	};
};


class CfgVehicles
{
	class MushroomsStageTransitions;
	class FoodAnimationSources;
	class Inventory_Base;
	class MushroomBase;
	class Edible_Base;

	//! Berries 
	class ZenSkillsBerryBase: MushroomBase
	{
		scope=0;
		debug_ItemCategory=6;
		weight=1;
		itemSize[]={1,1};
		stackedUnit="g";
		absorbency=0.40000001;
		varQuantityInit=50;
		varQuantityMin=0;
		varQuantityMax=50;
		quantityBar=1;
		inventorySlot[]=
		{
			"Ingredient"
		};
		containsSeedsType="";
		containsSeedsQuantity="0";
		hiddenSelections[]=
		{
			"cs_raw",
			"cs_dried"
		};
		hiddenSelectionsTextures[]=
		{
			"dz\gear\food\data\sambucus_nigra_CO.paa",
			"dz\gear\food\data\sambucus_nigra_dried_CO.paa",
			"dz\gear\food\data\sambucus_nigra_dried_CO.paa",
			"dz\gear\food\data\sambucus_nigra_dried_CO.paa",
			"dz\gear\food\data\sambucus_nigra_burnt_CO.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"dz\gear\food\data\sambucus_nigra_raw.rvmat",
			"dz\gear\food\data\sambucus_nigra_boiled.rvmat",
			"dz\gear\food\data\sambucus_nigra_baked.rvmat",
			"dz\gear\food\data\sambucus_nigra_dried.rvmat",
			"dz\gear\food\data\sambucus_nigra_burnt.rvmat",
			"dz\gear\food\data\sambucus_nigra_rotten.rvmat"
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=10;
					healthLevels[]=
					{
						
						{
							1,
							{}
						},
						
						{
							0.69999999,
							{}
						},
						
						{
							0.5,
							{}
						},
						
						{
							0.30000001,
							{}
						},
						
						{
							0,
							{}
						}
					};
				};
			};
		};
		class AnimationSources: FoodAnimationSources
		{
		};
		// Copied from AgaricusMushroom
		class Food
		{
			class FoodStages
			{
				class Raw
				{
					visual_properties[]={0,0,0};
					nutrition_properties[]={2.5,120,100,1,0};
					cooking_properties[]={0,0};
				};
				class Rotten
				{
					visual_properties[]={-1,-1,5};
					nutrition_properties[]={2,90,40,1,0,16};
					cooking_properties[]={0,0};
				};
				class Baked
				{
					visual_properties[]={0,1,1};
					nutrition_properties[]={1.75,300,60,1,0};
					cooking_properties[]={70,35};
				};
				class Boiled
				{
					visual_properties[]={0,2,2};
					nutrition_properties[]={1.5,250,160,1,0};
					cooking_properties[]={105,45};
				};
				class Dried
				{
					visual_properties[]={1,3,3};
					nutrition_properties[]={0.75,250,20,1,0};
					cooking_properties[]={70,30,80};
				};
				class Burned
				{
					visual_properties[]={0,4,4};
					nutrition_properties[]={2,90,0,1,0};
					cooking_properties[]={100,20};
				};
			};
			class FoodStageTransitions: MushroomsStageTransitions
			{
			};
		};
		class Trapping
		{
			baitTypes[]={5};
			baitTypeChances[]={0.60000002};
			resultQuantityBaseMod=0;
			resultQuantityDispersionMin=0;
			resultQuantityDispersionMax=0;
		};
		soundImpactType="organic";
		class AnimEvents
		{
			class SoundWeapon
			{
				class openTunaCan
				{
					soundSet="openTunaCan_SoundSet";
					id=204;
				};
				class pickUpItem
				{
					soundSet="Zucchini_pickup_SoundSet";
					id=797;
				};
				class Eating_TakeFood
				{
					soundSet="Eating_TakeFood_Soundset";
					id=889;
				};
				class Eating_BoxOpen
				{
					soundSet="Eating_BoxOpen_Soundset";
					id=893;
				};
				class Eating_BoxShake
				{
					soundSet="Eating_BoxShake_Soundset";
					id=894;
				};
				class Eating_BoxEnd
				{
					soundSet="Eating_BoxEnd_Soundset";
					id=895;
				};
			};
		};
	};
	class ZenSkills_SambucusBerry: ZenSkillsBerryBase
	{
		scope=2;
		displayName="$STR_SambucusBerry0";
		descriptionShort="$STR_SambucusBerry1";
		model="\dz\gear\food\Sambucus_nigra.p3d";
	};
	class ZenSkills_CaninaBerry: ZenSkillsBerryBase
	{
		scope=2;
		displayName="$STR_CaninaBerry0";
		descriptionShort="$STR_CaninaBerry1";
		model="\dz\gear\food\canina_raw.p3d";
	};

	// Books 
	class ZenSkills_BookBase: Inventory_Base
	{
		scope=0;
		rotationFlags=16;
		model="\DZ\gear\books\Book_kniga.p3d";
		inventorySlot[]=
		{
			"Book"
		};
		itemSize[]={2,2};
		absorbency=0.89999998;
		hiddenSelections[]=
		{
			"camoGround"
		};
		hiddenSelectionsTextures[]=
		{
			"dz\gear\books\data\book_bible_CO.paa"
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=100;
					healthLevels[]=
					{
						
						{
							1,
							
							{
								"DZ\gear\books\Data\book.rvmat"
							}
						},
						
						{
							0.69999999,
							
							{
								"DZ\gear\books\Data\book.rvmat"
							}
						},
						
						{
							0.5,
							
							{
								"DZ\gear\books\Data\book_damage.rvmat"
							}
						},
						
						{
							0.30000001,
							
							{
								"DZ\gear\books\Data\book_damage.rvmat"
							}
						},
						
						{
							0,
							
							{
								"DZ\gear\books\Data\book_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class ZenSkills_Book_Random: ZenSkills_BookBase
	{
		scope=2;
		displayName="$STR_ZenSkills_Item_SkillBook";
		descriptionShort="$STR_ZenSkills_Item_SkillBook_Desc";
		hiddenSelectionsTextures[]=
		{
			"ZenSkills\data\textures\books\template.paa"
		};
	};
	class ZenSkills_Book_Survival: ZenSkills_BookBase
	{
		scope=2;
		displayName="$STR_ZenSkills_Item_SurvivalBook";
		descriptionShort="$STR_ZenSkills_Item_SurvivalBook_Desc";
		hiddenSelectionsTextures[]=
		{
			"ZenSkills\data\textures\books\survival.paa"
		};
	};
	class ZenSkills_Book_Crafting: ZenSkills_BookBase
	{
		scope=2;
		displayName="$STR_ZenSkills_Item_CraftingBook";
		descriptionShort="$STR_ZenSkills_Item_CraftingBook_Desc";
		hiddenSelectionsTextures[]=
		{
			"ZenSkills\data\textures\books\crafting.paa"
		};
	};
	class ZenSkills_Book_Hunting: ZenSkills_BookBase
	{
		scope=2;
		displayName="$STR_ZenSkills_Item_HuntingBook";
		descriptionShort="$STR_ZenSkills_Item_HuntingBook_Desc";
		hiddenSelectionsTextures[]=
		{
			"ZenSkills\data\textures\books\hunting.paa"
		};
	};
	class ZenSkills_Book_Gathering: ZenSkills_BookBase
	{
		scope=2;
		displayName="$STR_ZenSkills_Item_GatheringBook";
		descriptionShort="$STR_ZenSkills_Item_GatheringBook_Desc";
		hiddenSelectionsTextures[]=
		{
			"ZenSkills\data\textures\books\gathering.paa"
		};
	};

	class ZenSkills_Injector_Base: Inventory_Base
	{
		scope=0;
		model="\dz\gear\medical\Morphine.p3d";
		rotationFlags=17;
		itemSize[]={1,2};
		weight=60;
		soundImpactType="plastic";
		hiddenSelections[]=
		{
			"zbytek"
		};
	};
	class ZenSkills_Injector_ExpBoost: ZenSkills_Injector_Base
	{
		scope=2;
		displayName="$STR_ZenSkills_Item_ExpBooster";
		descriptionShort="$STR_ZenSkills_Item_ExpBooster_Desc";
		hiddenSelectionsTextures[]=
		{
			"ZenSkills\data\textures\injectors\injector_boost_co.paa"
		};
	};
	class ZenSkills_Injector_PerkReset: ZenSkills_Injector_Base
	{
		scope=2;
		displayName="$STR_ZenSkills_Item_PerkResetter";
		descriptionShort="$STR_ZenSkills_Item_PerkResetter_Desc";
		hiddenSelectionsTextures[]=
		{
			"ZenSkills\data\textures\injectors\injector_perk_reset_co.paa"
		};
	};
	
	class ZenSkills_Injector_AdminDebugTool_MaxEXP: ZenSkills_Injector_Base
	{
		scope=2;
		displayName="ADMIN TOOL - GIVE MAX EXP";
		descriptionShort="NOT FOR GENERAL CONSUMPTION";
		hiddenSelectionsTextures[]=
		{
			"ZenSkills\data\textures\injectors\injector_boost_co.paa"
		};
	};
	class ZenSkills_Injector_AdminDebugTool_RemoveEXP: ZenSkills_Injector_Base
	{
		scope=2;
		displayName="ADMIN TOOL - REMOVE ALL EXP";
		descriptionShort="NOT FOR GENERAL CONSUMPTION";
		hiddenSelectionsTextures[]=
		{
			"ZenSkills\data\textures\injectors\injector_perk_reset_co.paa"
		};
	};
};

// Sound file config
class CfgSoundShaders
{
	class ZenSkillsGUIBase_SoundShader
	{
		frequency = 1;
		range = 10;
		volume = 1;
	};
	class ZenSkills_Click_SoundShader : ZenSkillsGUIBase_SoundShader
	{
		samples[] =
		{
			{ "ZenSkills\data\sounds\click", 1 }
		};
	};
	class ZenSkills_PerkUnlocked_SoundShader : ZenSkillsGUIBase_SoundShader
	{
		samples[] =
		{
			{ "ZenSkills\data\sounds\perkunlock1", 1 }
		};
	};
	class ZenSkills_PerkUnlockedMax_SoundShader : ZenSkillsGUIBase_SoundShader
	{
		samples[] =
		{
			{ "ZenSkills\data\sounds\perkunlock2", 1 }
		};
	};
	class ZenSkills_PerkNotify_SoundShader : ZenSkillsGUIBase_SoundShader
	{
		samples[] =
		{
			{ "ZenSkills\data\sounds\perknotify", 1 }
		};
	};
	class ZenSkills_PerkReset_SoundShader : ZenSkillsGUIBase_SoundShader
	{
		samples[] =
		{
			{ "ZenSkills\data\sounds\perkreset", 1 }
		};
	};
};

// Sound play config
class CfgSoundSets
{
	class ZenSkillsGUIBase_SoundSet
	{ 
		spatial = 0;
	};
	class ZenSkillsGUI_Click_SoundSet : ZenSkillsGUIBase_SoundSet
	{
		soundShaders[]=
		{
			"ZenSkills_Click_SoundShader"
		};
		volumeFactor = 0.25;
	};
	class ZenSkillsGUI_PerkUnlocked_SoundSet : ZenSkillsGUIBase_SoundSet
	{
		soundShaders[]=
		{
			"ZenSkills_PerkUnlocked_SoundShader"
		};
		volumeFactor = 0.5;
	};
	class ZenSkillsGUI_PerkUnlockedMax_SoundSet : ZenSkillsGUIBase_SoundSet
	{
		soundShaders[]=
		{
			"ZenSkills_PerkUnlockedMax_SoundShader"
		};
		volumeFactor = 0.5;
	};
	class ZenSkillsGUI_PerkNotify_SoundSet : ZenSkillsGUIBase_SoundSet
	{
		soundShaders[]=
		{
			"ZenSkills_PerkNotify_SoundShader"
		};
		volumeFactor = 0.5;
	};
	class ZenSkillsGUI_PerkReset_SoundSet : ZenSkillsGUIBase_SoundSet
	{
		soundShaders[]=
		{
			"ZenSkills_PerkReset_SoundShader"
		};
		volumeFactor = 0.5;
	};
};
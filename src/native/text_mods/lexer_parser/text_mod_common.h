//
// Date       : 17/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

////////////////////////////////////////////////////////////////////////////////
// | INCLUDES |
////////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "text_mod_utils.h"

////////////////////////////////////////////////////////////////////////////////
// | DEFINES |
////////////////////////////////////////////////////////////////////////////////

#define TXT_LOG(msg, ...) std::cout << std::format("[{}] " msg "\n", __FUNCTION__, __VA_ARGS__)

#ifdef _MSC_VER
#define TXT_ASSERT_FAIL __debugbreak()
#else
TXT_ASSERT_FAIL assert(false)
#endif

#define TXT_MOD_ASSERT(p, msg, ...)    \
    do {                               \
        if (!(p)) {                    \
            TXT_LOG(msg, __VA_ARGS__); \
            TXT_ASSERT_FAIL;           \
        }                              \
    } while (false)

////////////////////////////////////////////////////////////////////////////////
// | TEXT MODS |
////////////////////////////////////////////////////////////////////////////////

namespace bl1_text_mods {

using std::size_t;

// clang-format off

#ifdef TEXT_MODS_USE_WCHAR

using str      = std::wstring;
using str_view = std::wstring_view;
using txt_char = wchar_t;

#define TXT(str) L ## str

#else

using str      = std::string;
using str_view = std::string_view;
using txt_char = char;

#define TXT(str) str

#endif

// clang-format on

////////////////////////////////////////////////////////////////////////////////
// | TEST DATA |
////////////////////////////////////////////////////////////////////////////////

constexpr str_view test_str = TXT(R"(
/*////*/
# A comment
set Foo.Bar.Baz MyProperty 50
set Foo.Bar.Baz MyProperty "Some String"

set Foo.Bar.Baz MyProperty (A=(R=1,G=), B=, C="My String")
set Foo.Bar.Baz MyProperty (
    A=(
        B=10,
        C=20,
        D=()
    )
)

/* Multi
   Line
   Comment
*/


set Class Property ()
set Class Property
set Class Property 3.1415
set Class Property 3.1415.123

set Class Property ( A = None
                   , B =
                   , C = SomeClassType'Path.To.The.Object:Child_Ref_0'
                   )

set GD_Weap_SniperRifles.A_Weapons.WeaponType_Hyperion_Sniper FireRate 0.500000

##

set GD_Lobelia_ClassMods
    .A_Item_Soldier
    .ClassMod_Soldier_LegendaryPointman
    :AttributePresentationDefinition_0
    Description "My rifle, my pony, and me."

set GD_Weap_Launchers.Barrel.L_Barrel_Torgue_Nukem BoneControllers ((BoneName="Vladof_Pods",ControlType=WEAP_BONE_CONTROL_BarrelSpinner,bUseInFirstPerson=True,bUseInThirdPerson=True,ControlTemplate=WillowSkelControlLerpSingleBone'GD_Weap_Launchers.Barrel.L_Barrel_Vladof_Mongol:WillowSkelControlLerpSingleBone_0'))

set Aster_GunMaterials.Materials.SMG.Mati_BanditUniqueSMG_Orc VectorParameterValues ((ParameterName="p_AColorHilight",ParameterValue=(R=1.300000,G=1.300000,B=1.500000,A=1.000000),ExpressionGUID=(A=170714760,B=1132476783,C=-275668290,D=655702143)),(ParameterName="p_AColorMidtone",ParameterValue=(R=1.400000,G=1.400000,B=1.500000,A=1.000000),ExpressionGUID=(A=473594356,B=1338758895,C=824823946,D=864253813)),(ParameterName="p_AColorShadow",ParameterValue=(R=0.800000,G=0.800000,B=1.000000,A=1.000000),ExpressionGUID=(A=-429590341,B=1156435294,C=-1015192901,D=687313413)),(ParameterName="p_BColorHilight",ParameterValue=(R=1.000000,G=0.000000,B=0.000000,A=1.000000),ExpressionGUID=(A=170714760,B=1132476783,C=-275668290,D=655702143)),(ParameterName="p_BColorMidtone",ParameterValue=(R=0.800000,G=0.000000,B=0.000000,A=1.000000),ExpressionGUID=(A=473594356,B=1338758895,C=824823946,D=864253813)),(ParameterName="p_BColorShadow",ParameterValue=(R=0.400000,G=0.000000,B=0.000000,A=1.000000),ExpressionGUID=(A=-429590341,B=1156435294,C=-1015192901,D=687313413)),(ParameterName="p_CColorHilight",ParameterValue=(R=0.090000,G=0.090000,B=0.090000,A=1.000000),ExpressionGUID=(A=170714760,B=1132476783,C=-275668290,D=655702143)),(ParameterName="p_CColorMidtone",ParameterValue=(R=0.020000,G=0.020000,B=0.020000,A=1.000000),ExpressionGUID=(A=473594356,B=1338758895,C=824823946,D=864253813)),(ParameterName="p_CColorShadow",ParameterValue=(R=0.005000,G=0.005000,B=0.005000,A=1.000000),ExpressionGUID=(A=-429590341,B=1156435294,C=-1015192901,D=687313413)),(ParameterName="p_ReflectColor",ParameterValue=(R=1.300000,G=1.300000,B=1.500000,A=1.000000),ExpressionGUID=(A=295058103,B=1318551573,C=-2045449573,D=-547597976)),(ParameterName="p_ReflectionChannelScale",ParameterValue=(R=1.000000,G=1.000000,B=0.400000,A=1.000000),ExpressionGUID=(A=1869386622,B=1303200947,C=-1616405849,D=714558284)),(ParameterName="p_DColor",ParameterValue=(R=0.077345,G=0.077345,B=0.077345,A=1.000000),ExpressionGUID=(A=696455109,B=1155878830,C=-1741888361,D=802120528)),(ParameterName="p_EmissiveColor",ParameterValue=(R=1.500000,G=1.000000,B=0.300000,A=1.000000),ExpressionGUID=(A=-2074486426,B=1296399582,C=-2021314681,D=-350758005)),(ParameterName="p_DecalScalePosition",ParameterValue=(R=4.950000,G=10.285500,B=0.423200,A=0.504300),ExpressionGUID=(A=395540170,B=1243133493,C=-1264190552,D=123075385)),(ParameterName="p_DecalColor",ParameterValue=(R=5.000000,G=5.000000,B=5.000000,A=1.000000),ExpressionGUID=(A=1691998600,B=1239094551,C=2074257317,D=1844701893)),(ParameterName="p_PatternScalePosition",ParameterValue=(R=0.000000,G=0.000000,B=-0.000000,A=0.000000),ExpressionGUID=(A=-2005018406,B=1132497243,C=-39915121,D=208423616)),(ParameterName="p_DecalChannel",ParameterValue=(R=1.000000,G=1.000000,B=1.000000,A=1.000000),ExpressionGUID=(A=1757499073,B=1097055033,C=-1266029657,D=1038353636)),(ParameterName="p_PatternChannelScale",ParameterValue=(R=0.000000,G=0.000000,B=0.000000,A=0.000000),ExpressionGUID=(A=439432319,B=1091149893,C=-1991909502,D=1816944627)),(ParameterName="p_PatternColor",ParameterValue=(R=1.000000,G=1.000000,B=1.000000,A=1.000000),ExpressionGUID=(A=676539706,B=1125682796,C=1871983293,D=-2049503601)))

set Foo.Baz:Bar My_Cool_Property (A=-1.0,B=-1,C=-1.0)

-1
-2.1
-01.0

Begin Object Class=WeaponPartDefinition Name=barrel5_Jakobs_Unforgiven
   Begin Object Class=Behavior_CauseDamage Name=Behavior_CauseDamage_0
      DamageFormula=(BaseValueConstant=250.000000)
      Name="Behavior_CauseDamage_0"
      ObjectArchetype=Behavior_CauseDamage'WillowGame.Default__Behavior_CauseDamage'
   End Object
   PartNumberAddend=1
   TitleList(0)=WeaponNamePartDefinition'gd_weap_revolver_pistol.Title.TitleM_Jakobs1_Unforgiven'
   CashValueModifier=(BaseValueConstant=10.000000)
   Rarity=(BaseValueAttribute=InventoryAttributeDefinition'gd_Balance_Inventory.Rarity_Weapon.WeaponPartRarity1_Common')
   ExternalAttributeEffects(0)=(AttributeToModify=ResourcePoolAttributeDefinition'd_attributes.AccuracyResourcePool.AccuracyMaxValue',BaseModifierValue=(BaseValueConstant=-0.400000,BaseValueScaleConstant=1.000000))
   ExternalAttributeEffects(1)=(AttributeToModify=ResourcePoolAttributeDefinition'd_attributes.AccuracyResourcePool.AccuracyMinValue',BaseModifierValue=(BaseValueConstant=-2.000000,BaseValueScaleConstant=1.000000))
   ExternalAttributeEffects(2)=(AttributeToModify=ResourcePoolAttributeDefinition'd_attributes.AccuracyResourcePool.AccuracyOnIdleRegenerationRate',BaseModifierValue=(BaseValueConstant=-0.700000,BaseValueScaleConstant=1.000000))
   ExternalAttributeEffects(3)=(AttributeToModify=AttributeDefinition'd_attributes.GameplayAttributes.PlayerCriticalHitBonus',ModifierType=MT_PreAdd,BaseModifierValue=(BaseValueConstant=1.000000,BaseValueScaleConstant=1.000000))
   WeaponAttributeEffects(0)=(AttributeToModify=AttributeDefinition'd_attributes.Weapon.WeaponDamage',BaseModifierValue=(BaseValueConstant=0.500000,BaseValueScaleConstant=1.000000))
   WeaponAttributeEffects(1)=(AttributeToModify=AttributeDefinition'd_attributes.Weapon.WeaponPerShotAccuracyImpulse',BaseModifierValue=(BaseValueConstant=0.500000,BaseValueScaleConstant=1.000000))
   WeaponAttributeEffects(2)=(AttributeToModify=AttributeDefinition'd_attributes.Weapon.WeaponSpread',BaseModifierValue=(BaseValueConstant=-1.300000,BaseValueScaleConstant=1.000000))
   WeaponAttributeEffects(3)=(AttributeToModify=AttributeDefinition'd_attributes.Weapon.WeaponFireRate',BaseModifierValue=(BaseValueConstant=0.350000,BaseValueScaleConstant=1.000000))
   WeaponCardAttributes(0)=(Attribute=AttributeDefinition'd_attributes.GameplayAttributes.PlayerCriticalHitBonus',PriorityIncrease=3.000000)
   PartType=WP_Barrel
   SkeletalMesh=SkeletalMesh'weap_revolver_pistol.Barrel.barrel5'
   bIsGestaltMode=True
   GestaltModeSkeletalMeshName="barrel5"
   WeaponBehaviors=(OnPlayerEquip=(Behavior_CauseDamage'gd_weap_revolver_pistol.Barrel.barrel5_Jakobs_Unforgiven:Behavior_CauseDamage_0'))
   Name="barrel5_Jakobs_Unforgiven"
   ObjectArchetype=WeaponPartDefinition'WillowGame.Default__WeaponPartDefinition'
End Object

)");

}  // namespace bl1_text_mods
# Text Mods Lexer & Parser

This is a basic tokeniser and parser for a simple DSL language. This does not fully handle everything
since I don't fully know what the requirements are.

## Language Definition

### Comments

These are non-standard and are built into the lexer effectively they are escape sequences that we 
will ignore during processing.

```text
# Single Line Comment

/*
  Multi-line
  Comment
*/

```

### BL2 Style Set Commands

I am not very knowledgeable on BL2 modding, especially so for things like BLCMM and text mods in
general. So you should expect this to incorrect/wrong in a lot of places.

```text
set Object Property ValueExpression

# i.e.,

set GD_Weap_SniperRifles.A_Weapons.WeaponType_Hyperion_Sniper FireRate 0.500000
set Object Property (A=(), B=, C=1.0, D=Foo'Baz')

set Object Property (
    Color=(
        B=255,
        G=0,
        R=255,
        A=255
    ),
    X=,
    Y=(),
    Z(0)=10, // Dynamic array access assignment
    W[0]=10, // Static array access assignment
)
```

### BL2 Style Hotfixes (Not sure what these are called tbh)

The keywords level and none are reserved but currently unsure on how to handle these.

```text
level None set Object Property ValueExpression
```

### BL1 Object Definitions

The only reason for a complex Lexer & Parser is because I don't know if there is a builtin way to
handle object definitions like the one shown below. These definitions define not just the outermos 
object but all child objects as well.

These object definitions can be obtained using 'Copy to Clipboard' in the property view window for
the object in the unreal editor.

Things to note about these definitions
1. They can refer to an existing object, in which, we mutate the object
2. The definition may contain child definitions which may or may not exist
3. There is no good way to get the package from the object definition so we will extend the 
    definition to include the package. Child object definitions can rely on the parent for the package

```text
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
```
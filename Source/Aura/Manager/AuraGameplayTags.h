#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AuraGameplayTags
 *
 * 싱글톤 패턴으로 선언된 구조체입니다.
 * AssetManager가 자신의 Init 타이밍에 맞춰 함께 이 구조체의 Init 함수를 호출합니다.
 * 런타임 중 Tag에 접근 가능하도록 선언 및 초기화하는 역할이며, 일부 디자이너와 깊게 관련된 태그를 제외하면 대부분 여기에서 선언됩니다.
 * 여기에 존재하지 않는 태그는 Project Settings에서 선언합니다.
 */

struct FAuraGameplayTags
{
	static const FAuraGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	//~ Begin Attributes
	FGameplayTag Attributes_Vital_Health;
	FGameplayTag Attributes_Vital_Mana;
	
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;
	
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;

	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning;
	FGameplayTag Attributes_Resistance_Arcane;
	FGameplayTag Attributes_Resistance_Physical;
	
	FGameplayTag Attributes_Meta_IncomingXP;
	//~ End Attributes

	//~ Begin Input Tags
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_Passive_1;
	FGameplayTag InputTag_Passive_2;
	//~ End Input Tags

	//~ Begin Damage Types
	FGameplayTag Damage;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Arcane;
	FGameplayTag Damage_Physical;

	// 데미지 타입과 Resistance 타입을 묶는 TMap
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
	//~ End Damage Types

	//~ Begin Debuff Types
	FGameplayTag Debuff_Types_Burn;
	FGameplayTag Debuff_Types_Stun;
	FGameplayTag Debuff_Types_Confuse;

	FGameplayTag Debuff_Damage;
	//~ End Debuff Types
	
	//~ Begin Abilities
	FGameplayTag Abilities_None;
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_HitReact;
	FGameplayTag Abilities_Summon;
	
	FGameplayTag Abilities_Fire_FireBolt;
	FGameplayTag Abilities_Fire_FireRain;
	FGameplayTag Abilities_Lighting_Electrocute;

	FGameplayTag Abilities_Passive_HaloOfProtection;
	FGameplayTag Abilities_Passive_LifeSiphon;
	FGameplayTag Abilities_Passive_ManaSiphon;
	//~ End Abilities

	//~ Begin Cooldown
	FGameplayTag Cooldown_Fire_FireBolt;
	FGameplayTag Cooldown_Fire_FireRain;
	FGameplayTag Cooldown_Lighting_Electrocute;
	//~ End Cooldown

	//~ Begin Ability Status
	FGameplayTag Abilities_Status_Locked;
	FGameplayTag Abilities_Status_Eligible;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Equipped;
	//~ End AbilityStatus

	//~ Begin Ability Types
	FGameplayTag Abilities_Types_Active;
	FGameplayTag Abilities_Types_Passive;
	FGameplayTag Abilities_Types_None;
	//~ End Ability Types

	//~Begin Effects
	FGameplayTag Effects_HitReact;
	FGameplayTag Effects_GrantHitReact;
	//~ End Effects
	

	//~ Begin BehaviorTree
	FGameplayTag BT_Sub_Agro;
	FGameplayTag BT_Sub_Combat;
	//~ End BehaviorTree

	//~ Begin Player Block Tags
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;
	//~ End Player Block Tags

private:
	static FAuraGameplayTags GameplayTags;
};

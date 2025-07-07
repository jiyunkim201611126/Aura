#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AuraGameplayTags
 *
 * 싱글톤 패턴, Gameplay Tags를 보관
 */

struct FAuraGameplayTags
{
	static const FAuraGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	/** Attributes */
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
	/** End Attributes */

	/** Input Tags */
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	/** End Input Tags */

	/** Damage Type */
	FGameplayTag Damage;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Arcane;
	FGameplayTag Damage_Physical;

	// 데미지 타입과 Resistance 타입을 묶는 TMap
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
	/** End Damage Type */
	
	/** Common Abilities */
	// GameplayEffect가 이 태그를 갖고 있어야 HitReact가 발동됩니다.
	FGameplayTag Effects_GrantHitReact;
	FGameplayTag Effects_HitReact;
	/** End Common Abilities */

	/** Abilities */
	FGameplayTag Abilities_Attack;
	/** End Abilities */

	/** Combat Sockets */
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_RightHand;
	/** End Combat Sockets */

	/** Attack Montages */
	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;
	/** EndAttack Montages */

	/** Attack Sounds */
	FGameplayTag Sound_Attack_Swipe;
	/** End Attack Sounds */

	/** Projectile Sounds */
	FGameplayTag Sound_Projectile_Looping_FireBolt;
	
	FGameplayTag Sound_Projectile_Impact_FireBolt;
	FGameplayTag Sound_Projectile_Impact_Rock;
	/** End Projectile Sounds */

	/** Death Sounds */
	FGameplayTag Sound_Death_Goblin;
	FGameplayTag Sound_Death_Ghoul;
	/** End Death Sounds */

	/** Niagaras */
	FGameplayTag Niagara_BloodImpact_Red;
	FGameplayTag Niagara_BloodImpact_Green;
	
	FGameplayTag Niagara_Projectile_Impact_FireBolt;
	FGameplayTag Niagara_Projectile_Impact_Rock;
	/** End Niagaras */

private:
	static FAuraGameplayTags GameplayTags;
};

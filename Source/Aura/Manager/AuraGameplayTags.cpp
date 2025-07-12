#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"

FAuraGameplayTags FAuraGameplayTags::GameplayTags;

void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.Attributes_Vital_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.Health")), FString("Value needed to survive");
	GameplayTags.Attributes_Vital_Mana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.Mana")), FString("Value needed to use the skills");
	
	GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Strength")), FString("Increases physical damage");
	GameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Intelligence")), FString("Increases magical damage");
	GameplayTags.Attributes_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Resilience")), FString("Increases Armor and Armor Penetration");
	GameplayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Vigor")), FString("Increases Health");
	
	GameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Armor")), FString("Reduces damage taken, improves Block Chance");
	GameplayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ArmorPenetration")), FString("Ignores Percentage of enemy Armor, increases Critical Hit Chance");
	GameplayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.BlockChance")), FString("Chance to cut incoming damage in half");
	GameplayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitChance")), FString("Chance to double damage plus critical hit bonus");
	GameplayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitDamage")), FString("Bonus damage added when a critical hit is scored");
	GameplayTags.Attributes_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitResistance")), FString("Reduces Critical Hit Chance of attacking enemies");
	GameplayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.HealthRegeneration")), FString("Amount of Health regenerated every 1 second");
	GameplayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ManaRegeneration")), FString("Amount of Mana regenerated every 1 second");
	GameplayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxHealth")), FString("The maximum amount of Health obtainable");
	GameplayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxMana")), FString("The maximum amount of Mana obtainable");

	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"),FString("Input Tag for Left Mouse Button"));
	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"),FString("Input Tag for Right Mouse Button"));
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.1"),FString("Input Tag for 1 key"));
	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.2"),FString("Input Tag for 2 key"));
	GameplayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.3"),FString("Input Tag for 3 key"));
	GameplayTags.InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.4"),FString("Input Tag for 4 key"));

	
	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage"),FString("Damage"));

	// Damage Types
	
	GameplayTags.Damage_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Fire"),FString("Fire Damage Type"));
	GameplayTags.Damage_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Lightning"),FString("Lightning Damage Type"));
	GameplayTags.Damage_Arcane = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Arcane"),FString("Arcane Damage Type"));
	GameplayTags.Damage_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Physical"),FString("Physical Damage Type"));

	// Resistances
	
	GameplayTags.Attributes_Resistance_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Fire"),FString("Resistance to Fire Damage"));
	GameplayTags.Attributes_Resistance_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Lightning"),FString("Resistance to Lightning Damage"));
	GameplayTags.Attributes_Resistance_Arcane = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Arcane"),FString("Resistance to Arcane Damage"));
	GameplayTags.Attributes_Resistance_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Physical"),FString("Resistance to Physical Damage"));

	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Fire, GameplayTags.Attributes_Resistance_Fire);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Lightning, GameplayTags.Attributes_Resistance_Lightning);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Arcane, GameplayTags.Attributes_Resistance_Arcane);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical);

	
	GameplayTags.Effects_GrantHitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effects.GrantHitReact"),FString("Grant 'HitReact' tag if GameplayEffect has this tag"));
	GameplayTags.Effects_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effects.HitReact"),FString("Tag granted when Hit Reacting"));

	
	GameplayTags.Abilities_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Attack"),FString("Attack Ability Tag"));
	GameplayTags.Abilities_Summon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Summon"),FString("Summon Ability Tag"));

	// Sockets
	
	GameplayTags.CombatSocket_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.Weapon"),FString("Weapon"));
	GameplayTags.CombatSocket_LeftHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.LeftHand"),FString("Left Hand"));
	GameplayTags.CombatSocket_RightHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.RightHand"),FString("Right Hand"));
	GameplayTags.CombatSocket_Tail = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.Tail"),FString("Tail"));

	// Attack Sounds
	
	GameplayTags.Sound_Attack_Swipe = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Sound.Attack.Swipe"),FString("Swipe Sound"));

	// Projectile Sounds
	
	GameplayTags.Sound_Projectile_Looping_FireBolt = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Sound.Projectile.Looping.FireBolt"),FString("FireBolt Looping Sound"));
	
	GameplayTags.Sound_Projectile_Impact_FireBolt = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Sound.Projectile.Impact.FireBolt"),FString("FireBolt Impact Sound"));
	GameplayTags.Sound_Projectile_Impact_Rock = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Sound.Projectile.Impact.Rock"),FString("Rock Impact Sound"));

	// Death Sounds
	
	GameplayTags.Sound_Death_Goblin = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Sound.Death.Goblin"),FString("Goblin Death Sound"));
	GameplayTags.Sound_Death_Ghoul = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Sound.Death.Ghoul"),FString("Ghoul Death Sound"));
	GameplayTags.Sound_Death_Demon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Sound.Death.Demon"),FString("Demon Death Sound"));
	
	// Niagaras
	
	GameplayTags.Niagara_BloodImpact_Red = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Niagara.BloodImpact.Red"),FString("Red BloodImpact"));
	GameplayTags.Niagara_BloodImpact_Green = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Niagara.BloodImpact.Green"),FString("Green BloodImpact"));
	
	GameplayTags.Niagara_Projectile_Impact_FireBolt = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Niagara.Projectile.Impact.FireBolt"),FString("FireBolt Impact"));
	GameplayTags.Niagara_Projectile_Impact_Rock = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Niagara.Projectile.Impact.Rock"),FString("Rock Impact"));
}

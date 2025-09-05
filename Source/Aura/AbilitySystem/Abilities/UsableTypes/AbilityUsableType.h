#pragma once

#include "CoreMinimal.h"
#include "Aura/AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilityUsableType.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;
struct FGameplayAbilitySpec;
struct FGameplayAbilityActorInfo;
class UAuraAbilitySystemComponent;
class UAuraGameplayAbility;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class AURA_API UAbilityUsableType : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnEquipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC);
	virtual void OnUnequipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC);
	virtual bool CheckCost(const UAuraGameplayAbility* OwningAbility);
	virtual void ApplyCost(const UAuraGameplayAbility* OwningAbility);
	virtual void ActivateAbility(const UAuraGameplayAbility* OwningAbility);
	virtual void EndAbility(const UAuraGameplayAbility* OwningAbility);

	UFUNCTION(BlueprintPure, Category = "UsableType")
	virtual FText GetDescription();
};

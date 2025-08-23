#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilityUsableType.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;
struct FGameplayAbilitySpec;
struct FGameplayAbilityActorInfo;
class UAuraGameplayAbility;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class AURA_API UAbilityUsableType : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec, const UAuraGameplayAbility* OwningAbility);
	virtual bool CheckCost(const UAuraGameplayAbility* OwningAbility);
	virtual void ApplyCost(const UAuraGameplayAbility* OwningAbility);
	virtual void OnRemoveAbility(UAuraGameplayAbility* OwningAbility);

	UFUNCTION(BlueprintPure, Category = "UsableType")
	virtual FText GetDescription();
};

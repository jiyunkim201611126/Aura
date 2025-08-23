#pragma once

#include "CoreMinimal.h"
#include "StackableAbilityManager.h"
#include "Aura/AbilitySystem/Abilities/UsableTypes/AbilityUsableType.h"
#include "StackableAbility.generated.h"

struct FGameplayAbilityActorInfo;

UCLASS(BlueprintType, EditInlineNew)
class AURA_API UStackableAbility : public UAbilityUsableType
{
	GENERATED_BODY()

public:
	virtual void OnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec, const UAuraGameplayAbility* OwningAbility) override;
	virtual bool CheckCost(const UAuraGameplayAbility* OwningAbility) override;
	virtual void ApplyCost(const UAuraGameplayAbility* OwningAbility) override;
	virtual void OnRemoveAbility(UAuraGameplayAbility* OwningAbility) override;

	virtual FText GetDescription() override;

private:
	AStackableAbilityManager* GetStackableAbilityManager(const FGameplayAbilityActorInfo* ActorInfo) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FAbilityStackItem StackData;
};

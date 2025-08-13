#pragma once

#include "CoreMinimal.h"
#include "AbilityUsableType.h"
#include "Aura/Character/Component/StackableAbilityManager.h"
#include "StackableAbility.generated.h"

struct FGameplayAbilityActorInfo;

UCLASS(BlueprintType, EditInlineNew)
class AURA_API UStackableAbility : public UAbilityUsableType
{
	GENERATED_BODY()

public:
	virtual void OnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const UAuraGameplayAbility* OwningAbility) override;
	virtual void ApplyCost(const UAuraGameplayAbility* OwningAbility) override;
	virtual void OnRemoveAbility(UAuraGameplayAbility* OwningAbility) override;

private:
	AStackableAbilityManager* GetStackableAbilityManager(const FGameplayAbilityActorInfo* ActorInfo) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FAbilityStackItem StackData;
};

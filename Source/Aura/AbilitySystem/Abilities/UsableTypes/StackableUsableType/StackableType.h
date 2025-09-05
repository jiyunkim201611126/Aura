#pragma once

#include "CoreMinimal.h"
#include "StackableAbilityManager.h"
#include "Aura/AbilitySystem/Abilities/UsableTypes/AbilityUsableType.h"
#include "StackableType.generated.h"

class UAuraAbilitySystemComponent;
struct FGameplayAbilityActorInfo;

UCLASS(BlueprintType, EditInlineNew)
class AURA_API UStackableType : public UAbilityUsableType
{
	GENERATED_BODY()

public:
	virtual void OnEquipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC) override;
	virtual void OnUnequipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC) override;
	virtual bool CheckCost(const UAuraGameplayAbility* OwningAbility) override;
	virtual void ApplyCost(const UAuraGameplayAbility* OwningAbility) override;

	virtual FText GetDescription() override;
	
private:
	AStackableAbilityManager* GetStackableAbilityManager(const UAuraGameplayAbility* OwningAbility) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FAbilityStackItem StackData;
};

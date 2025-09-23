#pragma once

#include "CoreMinimal.h"
#include "StackableAbilityManager.h"
#include "Aura/AbilitySystem/Abilities/AbilityAdditionalCost/AbilityAdditionalCost.h"
#include "AbilityAdditionalCost_Stackable.generated.h"

class UAuraAbilitySystemComponent;
struct FGameplayAbilityActorInfo;

/**
 * 해당 객체를 소유하는 Ability는 스택형 Ability로 전환됩니다.
 * Instancing Policy가 Instanced Per Actor가 아닌 경우, ActorInfo가 nullptr이기 때문에 반드시 Instanced Per Actor로 설정해야 합니다. 
 */

UCLASS(BlueprintType, EditInlineNew)
class AURA_API UAbilityAdditionalCost_Stackable : public UAbilityAdditionalCost
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
	// Manager 클래스의 AbilityStackItem을 그대로 사용하는 것이 아닌, 독자적으로 FScalableFloat을 선언하는 방법으로 레벨별 StackData를 따로 구성할 수도 있습니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FAbilityStackItem StackData;
};

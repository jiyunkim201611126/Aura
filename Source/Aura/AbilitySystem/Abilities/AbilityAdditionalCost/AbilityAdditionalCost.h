#pragma once

#include "CoreMinimal.h"
#include "Aura/AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilityAdditionalCost.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;
struct FGameplayAbilitySpec;
struct FGameplayAbilityActorInfo;
class UAuraAbilitySystemComponent;
class UAuraGameplayAbility;

/**
 * Ability의 추가적인 Cost를 구현하는 클래스입니다.
 * OnEquip, OnUnequip에서 관련 Manager클래스에게 자신을 가진 Ability를 등록 및 해제합니다.
 * 그 뒤 Ability 발동 시마다 CheckCost, ApplyCost가 Ability의 함수와 함께 호출됩니다.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class AURA_API UAbilityAdditionalCost : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnEquipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC);
	virtual void OnUnequipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC);
	virtual bool CheckCost(const UAuraGameplayAbility* OwningAbility);
	virtual void ApplyCost(const UAuraGameplayAbility* OwningAbility);

	UFUNCTION(BlueprintPure, Category = "AdditionalCost")
	virtual FText GetDescription();
};

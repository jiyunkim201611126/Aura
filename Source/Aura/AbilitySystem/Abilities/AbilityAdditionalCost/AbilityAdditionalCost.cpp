#include "AbilityAdditionalCost.h"

void UAbilityAdditionalCost::OnEquipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC)
{
}

void UAbilityAdditionalCost::OnUnequipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC)
{
}

bool UAbilityAdditionalCost::CheckCost(const UAuraGameplayAbility* OwningAbility)
{
	return false;
}

void UAbilityAdditionalCost::ApplyCost(const UAuraGameplayAbility* OwningAbility)
{
}

void UAbilityAdditionalCost::ActivateAbility(const UAuraGameplayAbility* OwningAbility)
{
}

void UAbilityAdditionalCost::EndAbility(const UAuraGameplayAbility* OwningAbility)
{
}

FText UAbilityAdditionalCost::GetDescription()
{
	return FText();
}

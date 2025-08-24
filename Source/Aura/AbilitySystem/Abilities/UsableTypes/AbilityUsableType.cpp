#include "AbilityUsableType.h"

void UAbilityUsableType::OnEquipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC)
{
}

void UAbilityUsableType::OnUnequipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC)
{
}

bool UAbilityUsableType::CheckCost(const UAuraGameplayAbility* OwningAbility)
{
	return false;
}

void UAbilityUsableType::ApplyCost(const UAuraGameplayAbility* OwningAbility)
{
}

FText UAbilityUsableType::GetDescription()
{
	return FText();
}

#include "AbilityUsableType.h"

#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"

void UAbilityUsableType::OnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec, const UAuraGameplayAbility* OwningAbility)
{
}

bool UAbilityUsableType::CheckCost(const UAuraGameplayAbility* OwningAbility)
{
	return false;
}

void UAbilityUsableType::ApplyCost(const UAuraGameplayAbility* OwningAbility)
{
}

void UAbilityUsableType::OnRemoveAbility(UAuraGameplayAbility* OwningAbility)
{
}

FText UAbilityUsableType::GetDescription()
{
	return FText();
}

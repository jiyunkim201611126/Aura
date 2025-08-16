#include "StackableAbility.h"

#include "StackableAbilityManager.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/Abilities/AuraGameplayAbility.h"

void UStackableAbility::OnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec, const UAuraGameplayAbility* OwningAbility)
{
	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		if (AStackableAbilityManager* Manager = GetStackableAbilityManager(ActorInfo))
		{
			Manager->RegisterAbility(OwningAbility->AbilityTag, StackData.CurrentStack, StackData.MaxStack, StackData.RechargeTime);
		}
	}
}

bool UStackableAbility::CheckCost(const UAuraGameplayAbility* OwningAbility)
{
	// 충전된 스택이 없다면 false를 반환합니다.
	if (const AStackableAbilityManager* Manager = GetStackableAbilityManager(OwningAbility->GetCurrentActorInfo()))
	{
		if (!Manager->CheckCost(OwningAbility->AbilityTag))
		{
			return false;
		}
	}

	return true;
}

void UStackableAbility::ApplyCost(const UAuraGameplayAbility* OwningAbility)
{
	// 충전된 스택을 소모합니다.
	if (AStackableAbilityManager* Manager = GetStackableAbilityManager(OwningAbility->GetCurrentActorInfo()))
	{
		Manager->ApplyCost(OwningAbility->AbilityTag);
	}
}

void UStackableAbility::OnRemoveAbility(UAuraGameplayAbility* OwningAbility)
{
	// 이 Ability가 제거될 때, Component에서 이 Ability의 등록을 해제합니다.
	if (AStackableAbilityManager* Component = GetStackableAbilityManager(OwningAbility->GetCurrentActorInfo()))
	{
		Component->UnregisterAbility(OwningAbility->AbilityTag);
	}
}

AStackableAbilityManager* UStackableAbility::GetStackableAbilityManager(const FGameplayAbilityActorInfo* ActorInfo) const
{
	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(ActorInfo->AbilitySystemComponent);

	return ASC->FindOrAddAbilityManager<AStackableAbilityManager>();
}

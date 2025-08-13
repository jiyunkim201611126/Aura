#include "StackableAbility.h"

#include "Aura/AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Aura/Character/AuraCharacterBase.h"

void UStackableAbility::OnGivenAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		if (UStackableAbilityComponent* Component = GetStackableAbilityComponent(ActorInfo))
		{
			Component->RegisterAbility(Spec.Ability->GetAssetTags().First(), StackData.CurrentStack, StackData.MaxStack, StackData.RechargeTime);
		}
	}
}

bool UStackableAbility::CheckCost(const UAuraGameplayAbility* OwningAbility)
{
	// 충전된 스택이 없다면 false를 반환합니다.
	if (const UStackableAbilityComponent* Component = GetStackableAbilityComponent(OwningAbility->GetCurrentActorInfo()))
	{
		if (!Component->CheckCost(OwningAbility->GetAssetTags().First()))
		{
			return false;
		}
	}

	return true;
}

void UStackableAbility::ApplyCost(const UAuraGameplayAbility* OwningAbility)
{
	// 충전된 스택을 소모합니다.
	if (UStackableAbilityComponent* Component = GetStackableAbilityComponent(OwningAbility->GetCurrentActorInfo()))
	{
		Component->ApplyCost(OwningAbility->GetAssetTags().First());
	}
}

void UStackableAbility::OnRemoveAbility(UAuraGameplayAbility* OwningAbility)
{
	// 이 Ability가 제거될 때, Component에서 이 Ability의 등록을 해제합니다.
	if (UStackableAbilityComponent* Component = GetStackableAbilityComponent(OwningAbility->GetCurrentActorInfo()))
	{
		Component->UnregisterAbility(OwningAbility->GetAssetTags().First());
	}
}

UStackableAbilityComponent* UStackableAbility::GetStackableAbilityComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	// 이미 StackableAbilityComponent가 있다면 그 컴포넌트에 이 Ability를 등록하고, 없다면 서버일 때만 직접 스폰 후 붙여줍니다.
	// 클라이언트는 이후에 Component가 자동으로 복제됩니다.
	if (AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr)
	{
		UStackableAbilityComponent* Component = AvatarActor->FindComponentByClass<UStackableAbilityComponent>();
		if (Component)
		{
			return Component;
		}

		if (ActorInfo->IsNetAuthority())
		{
			Component = Cast<UStackableAbilityComponent>(AvatarActor->AddComponentByClass(UStackableAbilityComponent::StaticClass(), false, FTransform::Identity, true));
			
			if (AAuraCharacterBase* Character = Cast<AAuraCharacterBase>(AvatarActor))
			{
				Character->SetStackableAbilityComponent(Component);
			}
			
			AvatarActor->FinishAddComponent(Component, false, FTransform::Identity);
			return Component;
		}
	}

	return nullptr;
}

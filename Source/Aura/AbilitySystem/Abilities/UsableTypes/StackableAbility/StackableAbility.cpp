#include "StackableAbility.h"

#include "StackableAbilityManager.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Aura/Manager/AuraTextManager.h"

void UStackableAbility::OnEquipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC)
{
	// Equip 시점에선 아직 Ability가 제대로 초기화되지 않아 Ability를 통해 ASC를 추적하는 데에 실패할 가능성이 있습니다.
	if (AStackableAbilityManager* Manager = ASC->FindOrAddAbilityManager<AStackableAbilityManager>())
	{
		if (Manager)
		{
			Manager->RegisterAbility(OwningAbility->AbilityTag, StackData.CurrentStack, StackData.MaxStack, StackData.RechargeTime);
		}
	}
}

bool UStackableAbility::CheckCost(const UAuraGameplayAbility* OwningAbility)
{
	// 충전된 스택이 없다면 false를 반환합니다.
	if (const AStackableAbilityManager* Manager = GetStackableAbilityManager(OwningAbility))
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
	if (AStackableAbilityManager* Manager = GetStackableAbilityManager(OwningAbility))
	{
		Manager->ApplyCost(OwningAbility->AbilityTag);
	}
}

void UStackableAbility::OnRemoveAbility(UAuraGameplayAbility* OwningAbility)
{
	// 이 Ability가 제거될 때, Component에서 이 Ability의 등록을 해제합니다.
	if (AStackableAbilityManager* Manager = GetStackableAbilityManager(OwningAbility))
	{
		Manager->UnregisterAbility(OwningAbility->AbilityTag);
	}
}

FText UStackableAbility::GetDescription()
{
	return FText::Format(FAuraTextManager::GetText(EStringTableTextType::UI, FString("Abilities_Description_Stackable")), StackData.CurrentStack, StackData.MaxStack, StackData.RechargeTime);
}

AStackableAbilityManager* UStackableAbility::GetStackableAbilityManager(const UAuraGameplayAbility* OwningAbility) const
{
	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(OwningAbility->GetAbilitySystemComponentFromActorInfo());

	return ASC->FindOrAddAbilityManager<AStackableAbilityManager>();
}

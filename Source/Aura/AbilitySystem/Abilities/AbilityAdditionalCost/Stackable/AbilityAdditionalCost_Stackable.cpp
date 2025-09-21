#include "AbilityAdditionalCost_Stackable.h"

#include "StackableAbilityManager.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Aura/Manager/AuraTextManager.h"

void UAbilityAdditionalCost_Stackable::OnEquipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC)
{
	// Equip 시점에선 아직 Ability가 제대로 초기화되지 않아 Ability를 통해 ASC를 추적하는 데에 실패할 가능성이 있습니다.
	// 또, UCLASS 매크로에서 DefaultToInstanced를 사용하지 않았기 때문에 런타임 중 값에 변화를 줄 수 없습니다.
	// 따라서 매개변수로 들어오는 OwningAbility로 Manager에게 접근합니다.
	// DefaultToInstanced를 사용할 경우 Manager가 이 객체를 제대로 캐싱하지 못 하는 문제가 발생합니다.
	// ASC에 Ability가 부여되면서 자동으로 인스턴스가 복제되면서 발생하는 것으로 추측됩니다.
	if (AStackableAbilityManager* Manager = ASC->FindOrAddAbilityManager<AStackableAbilityManager>())
	{
		if (Manager)
		{
			Manager->RegisterAbility(OwningAbility->AbilityTag, StackData.CurrentStack, StackData.MaxStack, StackData.RechargeTime);
		}
	}
}

void UAbilityAdditionalCost_Stackable::OnUnequipAbility(const UAuraGameplayAbility* OwningAbility, UAuraAbilitySystemComponent* ASC)
{
	// 이 객체를 소유한 Ability를 장착 해제할 때, Component에서 이 Ability의 등록을 해제합니다.
	// Ability를 해제하는 게 아니라 Status 태그만 바꾸고 호출하는 타이밍인데도 ActorInfo가 정리되어있습니다.
	// 이유를 정확히 알 수는 없으나, GAS가 ActorInfo를 상당히 적극적으로 정리한다고 추측됩니다.
	if (AStackableAbilityManager* Manager = ASC->FindOrAddAbilityManager<AStackableAbilityManager>())
	{
		Manager->UnregisterAbility(OwningAbility->AbilityTag);
	}
}

bool UAbilityAdditionalCost_Stackable::CheckCost(const UAuraGameplayAbility* OwningAbility)
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

void UAbilityAdditionalCost_Stackable::ApplyCost(const UAuraGameplayAbility* OwningAbility)
{
	// 충전된 스택을 소모합니다.
	if (AStackableAbilityManager* Manager = GetStackableAbilityManager(OwningAbility))
	{
		Manager->ApplyCost(OwningAbility->AbilityTag);
	}
}

FText UAbilityAdditionalCost_Stackable::GetDescription()
{
	return FText::Format(FAuraTextManager::GetText(EStringTableTextType::UI, FString("Abilities.Description.Stackable")), StackData.CurrentStack, StackData.MaxStack, StackData.RechargeTime);
}

AStackableAbilityManager* UAbilityAdditionalCost_Stackable::GetStackableAbilityManager(const UAuraGameplayAbility* OwningAbility) const
{
	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(OwningAbility->GetAbilitySystemComponentFromActorInfo());

	return ASC->FindOrAddAbilityManager<AStackableAbilityManager>();
}

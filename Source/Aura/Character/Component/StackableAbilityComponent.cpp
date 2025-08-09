#include "StackableAbilityComponent.h"

UStackableAbilityComponent::UStackableAbilityComponent()
{
	SetIsReplicatedByDefault(true);
}

void UStackableAbilityComponent::DestroyComponent(bool bPromoteChildren)
{
	OnStackCountChanged.Unbind();
	OnStackTimerStarted.Unbind();
	Super::DestroyComponent(bPromoteChildren);
}

void UStackableAbilityComponent::RegisterAbility(FGameplayTag AbilityTag, int32 MaxStack, float RechargeTime)
{
	// Ability 등록 및 변수를 초기화합니다.
	FAbilityStackData& Data = AbilityStacks.FindOrAdd(AbilityTag);
	Data.CurrentStack = 0;
	Data.MaxStack = MaxStack;
	Data.RechargeTime = RechargeTime;

	StartRecharge(AbilityTag);
}

void UStackableAbilityComponent::UnregisterAbility(FGameplayTag AbilityTag)
{
	// 충전 로직을 중단, Ability를 엔트리에서 제외합니다.
	StopRecharge(AbilityTag);
	AbilityStacks.Remove(AbilityTag);
}

bool UStackableAbilityComponent::CheckCost(FGameplayTag AbilityTag) const
{
	const FAbilityStackData* Data = AbilityStacks.Find(AbilityTag);
	return Data && Data->CurrentStack > 0;
}

void UStackableAbilityComponent::ApplyCost(FGameplayTag AbilityTag)
{
	FAbilityStackData* Data = AbilityStacks.Find(AbilityTag);
	if (Data && Data->CurrentStack > 0)
	{
		Data->CurrentStack = FMath::Max(0, Data->CurrentStack - 1);

		// 스택을 소모했으므로 HUD에 알립니다.
		OnStackCountChanged.ExecuteIfBound(AbilityTag, Data->CurrentStack);
		StartRecharge(AbilityTag);
	}
}

int32 UStackableAbilityComponent::GetCurrentStack(FGameplayTag AbilityTag) const
{
	const FAbilityStackData* Data = AbilityStacks.Find(AbilityTag);
	return Data ? Data->CurrentStack : 0;
}

bool UStackableAbilityComponent::CheckHasAbility(FGameplayTag AbilityTag) const
{
	return AbilityStacks.Contains(AbilityTag);
}

void UStackableAbilityComponent::StartRecharge(FGameplayTag AbilityTag)
{
	FAbilityStackData* Data = AbilityStacks.Find(AbilityTag);

	// 최대 충전치인 경우 충전을 중단합니다.
	if (!Data || Data->CurrentStack >= Data->MaxStack)
	{
		StopRecharge(AbilityTag);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		// 최대 충전 상태가 아니고, 타이머가 돌고 있지 않은 경우 타이머를 작동시킵니다.
		if (!World->GetTimerManager().IsTimerActive(Data->RechargeTimerHandle))
		{
			World->GetTimerManager().SetTimer(
				Data->RechargeTimerHandle,
				FTimerDelegate::CreateUObject(this, &ThisClass::Recharge, AbilityTag),
				Data->RechargeTime,
				true);
			
			// 충전이 시작됐을을 HUD에 알립니다.
			OnStackTimerStarted.ExecuteIfBound(AbilityTag, Data->RechargeTime);
		}
	}
}

void UStackableAbilityComponent::StopRecharge(FGameplayTag AbilityTag)
{
	FAbilityStackData* Data = AbilityStacks.Find(AbilityTag);
	if (Data)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(Data->RechargeTimerHandle);
		}
	}
}

void UStackableAbilityComponent::Recharge(FGameplayTag AbilityTag)
{
	FAbilityStackData* Data = AbilityStacks.Find(AbilityTag);
	if (Data)
	{
		Data->CurrentStack++;

		// 스택이 충전됐으므로 HUD에 알립니다.
		OnStackCountChanged.ExecuteIfBound(AbilityTag, Data->CurrentStack);
		
		if (Data->CurrentStack >= Data->MaxStack)
		{
			StopRecharge(AbilityTag);
		}
		else
		{
			StartRecharge(AbilityTag);
			
			// 충전이 시작됐을을 HUD에 알립니다.
			OnStackTimerStarted.ExecuteIfBound(AbilityTag, Data->RechargeTime);
		}
	}
}

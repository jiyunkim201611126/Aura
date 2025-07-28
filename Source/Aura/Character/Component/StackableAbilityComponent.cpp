#include "StackableAbilityComponent.h"

void UStackableAbilityComponent::RegisterAbility(FGameplayTag AbilityTag, int32 MaxStack, float RechargeTime)
{
	FAbilityStackData& Data = AbilityStacks.FindOrAdd(AbilityTag);
	Data.CurrentStack = 0;
	Data.MaxStack = MaxStack;
	Data.RechargeTime = RechargeTime;
	StartRecharge(AbilityTag);
}

void UStackableAbilityComponent::UnregisterAbility(FGameplayTag AbilityTag)
{
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
		StartRecharge(AbilityTag);
	}
}

int32 UStackableAbilityComponent::GetCurrentStack(FGameplayTag AbilityTag) const
{
	const FAbilityStackData* Data = AbilityStacks.Find(AbilityTag);
	return Data ? Data->CurrentStack : 0;
}

void UStackableAbilityComponent::StartRecharge(FGameplayTag AbilityTag)
{
	FAbilityStackData* Data = AbilityStacks.Find(AbilityTag);
	if (!Data || Data->CurrentStack >= Data->MaxStack)
	{
		StopRecharge(AbilityTag);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(Data->RechargeTimerHandle))
		{
			World->GetTimerManager().SetTimer(
				Data->RechargeTimerHandle,
				FTimerDelegate::CreateUObject(this, &ThisClass::Recharge, AbilityTag),
				Data->RechargeTime,
				true);
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
		if (Data->CurrentStack >= Data->MaxStack)
		{
			StopRecharge(AbilityTag);
		}
	}
}

#include "WaitCooldownChange.h"

#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* InASC, const FGameplayTag& InCooldownTag)
{
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
	WaitCooldownChange->AbilitySystemComponent = InASC;
	WaitCooldownChange->CooldownTag = InCooldownTag;

	if (!IsValid(InASC) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	// Duration 기반으로 적용되는 GE가 부여됐을 때 호출되는 델리게이트에 함수를 바인드합니다.
	// 즉, Cooldown Tag 적용 시 호출될 함수를 바인드합니다.
	// 이 델리게이트는 서버와 클라이언트 모두 호출됩니다.
	InASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &ThisClass::OnActiveEffectAdded);

	// Cooldown Tag 제거 시 호출될 함수를 바인드합니다.
	InASC->RegisterGameplayTagEvent(InCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(WaitCooldownChange, &ThisClass::CooldownTagChanged);

	return WaitCooldownChange;
}

void UWaitCooldownChange::EndTask()
{
	// 바인드했던 함수를 제거합니다.
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}
	
	SetReadyToDestroy();
	MarkAsGarbage();
}

void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveGameplayEffect)
{
	// 부여된 GE 중 Cooldown Tag를 가진 GE를 찾습니다.
	if (TargetASC->HasMatchingGameplayTag(CooldownTag))
	{
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());

		// 찾은 GE의 적용 시간이 얼마나 남았는지 가져옵니다.
		TArray<float> TimesRemaining = AbilitySystemComponent->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			// 부여된 Cooldown Tag가 여러 개일 경우, 그 중 가장 수치가 큰 값을 찾는 과정입니다.
			float TimeRemaining = TimesRemaining[0];
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];;
				}

				// 남은 시간을 알립니다.
				CooldownStart.Broadcast();
			}
		}
	}
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		// 부여된 Tag가 제거된 경우 들어오는 분기입니다.
		CooldownEnd.Broadcast();
	}
}

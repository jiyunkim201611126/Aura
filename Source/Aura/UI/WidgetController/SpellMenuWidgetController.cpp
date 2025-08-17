#include "SpellMenuWidgetController.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/Data/AbilityInfo.h"
#include "Aura/Player/AuraPlayerState.h"

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	// Ability가 부여될 때, SpellMenuWidget이 이를 알 수 있도록 함수를 바인드합니다.
	// HUD에 대한 초기화가 모두 이루어지고 나서 GameAbility를 부여하기 때문에, 여기서 바인드하면 정상 작동합니다.
	if (GetAuraASC())
	{
		AuraAbilitySystemComponent->AbilitiesGivenDelegate.AddUObject(this, &ThisClass::OnAbilitiesGiven);
		AuraAbilitySystemComponent->AbilityStatusChangedDelegate.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const int32 AbilityLevel)
		{
			if (AbilityInfo)
			{
				FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				Info.StatusTag = StatusTag;
				AbilityInfoDelegate.Broadcast(Info);
			}
		});
	}

	if (GetAuraPS())
	{
		AuraPlayerState->OnSpellPointsChangedDelegate.AddLambda([this](int32 InSpellPoints)
		{
			OnSpellPointsChanged.Broadcast(InSpellPoints);
		});
	}
}

void USpellMenuWidgetController::BroadcastInitialValue()
{
	if (GetAuraPS())
	{
		OnSpellPointsChanged.Broadcast(AuraPlayerState->GetSpellPoints());
	}
}

void USpellMenuWidgetController::SpendPointButtonPressed(const FGameplayTag& AbilityTag)
{
	if (GetAuraASC())
	{
		AuraAbilitySystemComponent->ServerSpendSpellPoint(AbilityTag);
	}
}

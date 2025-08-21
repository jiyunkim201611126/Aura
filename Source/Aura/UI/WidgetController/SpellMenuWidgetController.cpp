#include "SpellMenuWidgetController.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/Data/AbilityInfo.h"
#include "Aura/Manager/AuraGameplayTags.h"
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
			if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
			{
				SelectedAbility.Status = StatusTag;
				ShouldEnableButtons();
			}
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
			CurrentSpellPoints = InSpellPoints;
			ShouldEnableButtons();
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

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipSelectionDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	if (GetAuraASC())
	{
		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		FGameplayTag AbilityStatus;

		const bool bTagValid = AbilityTag.IsValid();
		const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
		const FGameplayAbilitySpec* AbilitySpec = AuraAbilitySystemComponent->GetGivenAbilitySpecFromAbilityTag(AbilityTag);
		const bool bSpecValid = AbilitySpec != nullptr;
		
		if (!bTagValid || bTagNone || !bSpecValid)
		{
			// 잠겨있거나 아직 개발되지 않은 Ability인 경우 들어오는 분기
			AbilityStatus = GameplayTags.Abilities_Status_Locked;
		}
		else
		{
			AbilityStatus = AuraAbilitySystemComponent->GetStatusFromSpec(*AbilitySpec);
		}

		// 선택된 SpellGlobe의 Ability와 그 상태를 멤버변수로 캐싱합니다.
		SelectedAbility.Ability = AbilityTag;
		SelectedAbility.Status = AbilityStatus;

		// 버튼들 상태 조작을 시작합니다.
		ShouldEnableButtons();
	}
}

void USpellMenuWidgetController::ShouldEnableButtons()
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

	bool bShouldEnableSpellPointsButton = false;
	bool bShouldEnableEquipButton = false;
	if (SelectedAbility.Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (CurrentSpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (SelectedAbility.Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		if (CurrentSpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (SelectedAbility.Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		if (CurrentSpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	FText Description;
	FText NextLevelDescription;
	AuraAbilitySystemComponent->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription, AbilityInfo);

	OnSpellMenuStatusChangedDelegate.Broadcast(bShouldEnableSpellPointsButton, bShouldEnableEquipButton, Description, NextLevelDescription);
}

void USpellMenuWidgetController::SpendPointButtonPressed(const FGameplayTag& AbilityTag)
{
	if (GetAuraASC())
	{
		AuraAbilitySystemComponent->ServerSpendSpellPoint(AbilityTag);
	}
}

void USpellMenuWidgetController::GlobeDeselect()
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipSelectionDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	SelectedAbility.Ability = FAuraGameplayTags::Get().Abilities_None;
	SelectedAbility.Status = FAuraGameplayTags::Get().Abilities_Status_Locked;

	OnSpellMenuStatusChangedDelegate.Broadcast(false, false, FText(), FText());
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	WaitForEquipSelectionDelegate.Broadcast(AbilityType);
	bWaitingForEquipSelection = true;
}

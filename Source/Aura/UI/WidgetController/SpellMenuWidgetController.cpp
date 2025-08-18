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

				bool bEnableSpendPoints = false;
				bool bEnableEquip = false;
				ShouldEnableButtons(StatusTag, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
				OnSpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip);
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

			bool bEnableSpendPoints = false;
			bool bEnableEquip = false;
			ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
			OnSpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip);
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

		SelectedAbility.Ability = AbilityTag;
		SelectedAbility.Status = AbilityStatus;

		bool bEnableSpendPoints = false;
		bool bEnableEquip = false;
		ShouldEnableButtons(AbilityStatus, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
		OnSpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip);
	}
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

	bShouldEnableSpellPointsButton = false;
	bShouldEnableEquipButton = false;
	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
}

void USpellMenuWidgetController::SpendPointButtonPressed(const FGameplayTag& AbilityTag)
{
	if (GetAuraASC())
	{
		AuraAbilitySystemComponent->ServerSpendSpellPoint(AbilityTag);
	}
}

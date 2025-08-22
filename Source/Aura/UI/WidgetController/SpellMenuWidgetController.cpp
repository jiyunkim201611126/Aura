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
		AuraAbilitySystemComponent->OnAbilitiesGivenDelegate.AddUObject(this, &ThisClass::OnAbilitiesGiven);
		AuraAbilitySystemComponent->OnAbilityStatusChangedDelegate.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const int32 AbilityLevel)
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
		AuraAbilitySystemComponent->OnAbilityEquipped.AddUObject(this, &ThisClass::OnAbilityEquipped);
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

void USpellMenuWidgetController::SpellGlobeDeselected()
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

void USpellMenuWidgetController::SpendPointButtonPressed(const FGameplayTag& AbilityTag)
{
	if (GetAuraASC())
	{
		AuraAbilitySystemComponent->ServerSpendSpellPoint(AbilityTag);
	}
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	WaitForEquipSelectionDelegate.Broadcast(AbilityType);
	bWaitingForEquipSelection = true;

	// 이미 장착 Ability를 선택한 경우 들어가는 분기입니다.
	if (SelectedAbility.Status.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
	{
		// 어디에 장착되었는지 가져와 캐싱합니다.
		SelectedInputTag = GetAuraASC()->GetInputTagFromAbilityTag(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& InputTag, const FGameplayTag& AbilityType)
{
	if (!bWaitingForEquipSelection)
	{
		// 장착 대기 중 상태가 아니면 반환합니다.
		return;
	}

	// 장착을 위해 누른 슬롯이 장착을 위해 선택된 Ability의 타입(액티브, 패시브)와 다른 경우 반환합니다.
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	if (!SelectedAbilityType.MatchesTagExact(AbilityType))
	{
		return;
	}

	// 서버에 장착을 요청합니다.
	GetAuraASC()->ServerEquipAbility(SelectedAbility.Ability, InputTag);

	// UI에게 요청을 성공적으로 마쳤음을 알려줍니다.
	OnSuccessRequestEquipDelegate.Broadcast();
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

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& InputTag, const FGameplayTag& PreviousInputTag)
{
	bWaitingForEquipSelection = false;

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	// 변경 전 InputTag에 해당하는 슬롯이 비워지도록 비주얼을 업데이트합니다.
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousInputTag;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// 이번에 장착한 InputTag에 해당하는 Slot의 비주얼을 업데이트합니다.
	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = StatusTag;
	Info.InputTag = InputTag;
	AbilityInfoDelegate.Broadcast(Info);

	StopWaitingForEquipSelectionDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
}

#include "AuraWidgetController.h"
#include "Aura/Player/AuraPlayerController.h"
#include "Aura/Player/AuraPlayerState.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/AbilitySystem/Data/AbilityInfo.h"
#include "Aura/Manager/AuraGameplayTags.h"

void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WidgetControllerParams)
{
	PlayerController = WidgetControllerParams.PlayerController;
	PlayerState = WidgetControllerParams.PlayerState;
	AbilitySystemComponent = WidgetControllerParams.AbilitySystemComponent;
	AttributeSet = WidgetControllerParams.AttributeSet;
}

void UAuraWidgetController::BindCallbacksToDependencies()
{
}

void UAuraWidgetController::BroadcastInitialValue()
{
}

AAuraPlayerController* UAuraWidgetController::GetAuraPC()
{
	if (!AuraPlayerController)
	{
		AuraPlayerController = Cast<AAuraPlayerController>(PlayerController);
	}
	return AuraPlayerController;
}

AAuraPlayerState* UAuraWidgetController::GetAuraPS()
{
	if (!AuraPlayerState)
	{
		AuraPlayerState = Cast<AAuraPlayerState>(PlayerState);
	}
	return AuraPlayerState;
}

UAuraAbilitySystemComponent* UAuraWidgetController::GetAuraASC()
{
	if (!AuraAbilitySystemComponent)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	}
	return AuraAbilitySystemComponent;
}

UAuraAttributeSet* UAuraWidgetController::GetAuraAS()
{
	if (!AuraAttributeSet)
	{
		AuraAttributeSet = Cast<UAuraAttributeSet>(AttributeSet);
	}
	return AuraAttributeSet;
}

void UAuraWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag, const FGameplayTag& StatusTag, const FGameplayTag& PreviousInputTag)
{
	// 이 함수가 끝날 때까지 부여된 Ability의 목록을 변경하지 않도록 잠가주는 역할의 구문입니다.
	// 다른 곳에서 GiveAbility 등의 함수를 호출해도, 이 함수가 끝날 때까지 흐름이 보류됩니다.
	FScopedAbilityListLock ActiveScopeLock(*GetAuraASC());
	
	// 이전 InputTag에 해당하는 슬롯이 비워지도록 비주얼을 업데이트합니다.
	ClearSpellGlobe(PreviousInputTag);

	// 이번에 장착한 InputTag에 해당하는 Slot의 비주얼을 업데이트합니다.
	UpdateSpellGlobe(AbilityTag, InputTag, StatusTag);
}

void UAuraWidgetController::ClearSpellGlobe(const FGameplayTag& PreviousInputTag) const
{
	if (PreviousInputTag.IsValid())
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
		FAuraAbilityInfo LastSlotInfo;
		LastSlotInfo.InputTag = PreviousInputTag;
		LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
		LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
		AbilityInfoDelegate.Broadcast(LastSlotInfo);
	}
}

void UAuraWidgetController::UpdateSpellGlobe(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag, const FGameplayTag& StatusTag) const
{
	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.InputTag = InputTag;
	Info.StatusTag = StatusTag;
	AbilityInfoDelegate.Broadcast(Info);
}

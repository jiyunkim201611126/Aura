#include "AuraWidgetController.h"
#include "Aura/Player/AuraPlayerController.h"
#include "Aura/Player/AuraPlayerState.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/AbilitySystem/Data/AbilityInfo.h"

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

void UAuraWidgetController::OnAbilitiesGiven(const FGameplayAbilitySpec& AbilitySpec)
{
	if (GetAuraASC())
	{
		// 이 함수가 끝날 때까지 부여된 Ability의 목록을 변경하지 않도록 잠가주는 역할의 구문입니다.
		// 다른 곳에서 GiveAbility 등의 함수를 호출해도, 이 함수가 끝날 때까지 흐름이 보류됩니다.
		FScopedAbilityListLock ActiveScopeLock(*GetAuraASC());
		
		FAuraAbilityInfo AbilityUIInfo;
		MakeAbilityUIInfo(AbilitySpec, AbilityUIInfo);
		AbilityInfoDelegate.Broadcast(AbilityUIInfo);
	}
}

void UAuraWidgetController::MakeAbilityUIInfo(const FGameplayAbilitySpec& AbilitySpec, FAuraAbilityInfo& AbilityUIInfo)
{
	AbilityUIInfo = AbilityInfo->FindAbilityInfoForTag(GetAuraASC()->GetAbilityTagFromSpec(AbilitySpec));
	AbilityUIInfo.InputTag = AuraAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
	AbilityUIInfo.StatusTag = AuraAbilitySystemComponent->GetStatusFromSpec(AbilitySpec);
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

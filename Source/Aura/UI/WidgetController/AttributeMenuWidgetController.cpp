#include "AttributeMenuWidgetController.h"

#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/AbilitySystem/Data/AttributeInfo.h"
#include "Aura/Player/AuraPlayerState.h"

void UAttributeMenuWidgetController::BroadcastInitialValue()
{
	// 반드시 모든 바인드가 끝난 뒤에 호출할 것
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);

	for (auto& Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	OnAttributePointsChangedDelegate.Broadcast(AuraPlayerState->GetAttributePoints());
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	// 하나의 Attribute만 변경되어도 다른 Attribute에게 영향을 주므로 모두 새로고침해야 함
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);
	for (auto& Pair : AS->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}

	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	AuraPlayerState->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 NewAttributePoints)
		{
			OnAttributePointsChangedDelegate.Broadcast(NewAttributePoints);
		}
	);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	// Tag를 통해 Attribute의 Name, Description를 가져옵니다.
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	// Tag에 일치하는 값은 직접 AttributeSet에서 찾아 가져옵니다.
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}

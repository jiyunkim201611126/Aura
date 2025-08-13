#include "OverlayWidgetController.h"

#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/Data/AbilityInfo.h"
#include "Aura/Character/Component/StackableAbilityManager.h"
#include "Aura/Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValue()
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributeSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	AAuraPlayerState* AuraPlayerState = Cast<AAuraPlayerState>(PlayerState);
	AuraPlayerState->OnXPChangedDelegate.AddUObject(this, &ThisClass::OnXPChanged);
	AuraPlayerState->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
		}
	);
	
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	// 선언된 Attribute들에게 변동사항이 있는 경우 Widget Controller가 알 수 있도록 각 Attribute에게 함수를 바인드
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetManaAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxManaAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		// Ability가 부여될 때, OverlayWidget이 이를 알 수 있도록 함수를 바인드합니다.
		// HUD에 대한 초기화가 모두 이루어지고 나서 GameAbility를 부여하기 때문에, 여기서 바인드하면 정상 작동합니다.
		AuraASC->AbilitiesGivenDelegate.AddUObject(this, &ThisClass::OnAbilitiesGiven);

		// GameplayEffect가 적용될 때 화면에 메시지를 띄울 수 있도록 함수를 바인드합니다.
		AuraASC->EffectAssetTags.AddLambda([this](const FGameplayTagContainer& AssetTags)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
				
				// 상위 계층으로 물어보면 true, 하위 계층으로 물어보면 false를 반환함
				// "Message.HealthPotion".MatchesTag("Message")는 true를, "Message".MatchesTag("Message.HealthPotion")는 false를 반환
				if (Tag.MatchesTag(MessageTag))
				{
					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
					MessageWidgetRowDelegate.Broadcast(*Row);
				}
			}
		});
	}
}

void UOverlayWidgetController::OnAbilitiesGiven(const FGameplayAbilitySpec& AbilitySpec)
{
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		// 이 함수가 끝날 때까지 부여된 Ability의 목록을 변경하지 않도록 잠가주는 역할의 구문입니다.
		// 다른 곳에서 GiveAbility 등의 함수를 호출해도, 이 함수가 끝날 때까지 흐름이 보류됩니다.
		FScopedAbilityListLock ActiveScopeLock(*AuraASC);
		
		FAuraAbilityInfo AbilityUIInfo = AbilityInfo->FindAbilityInfoForTag(AuraASC->GetAbilityTagFromSpec(AbilitySpec));
		AbilityUIInfo.InputTag = AuraASC->GetInputTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(AbilityUIInfo);

		FAbilityUsableTypeInfo UsableTypeInfo;
		if (AStackableAbilityManager* StackableAbilityManager = AuraASC->GetStackableAbilityManager())
		{
			// Stackable Ability로 등록되어있는지 확인, 필요한 함수를 바인드합니다.
			if (StackableAbilityManager->CheckHasAbility(AbilityUIInfo.AbilityTag))
			{
				UsableTypeInfo.bIsStackable = true;
			}
			StackableAbilityManager->OnStackCountChanged.BindLambda(
				[this](FGameplayTag InAbilityTag,int32 StackCount)
				{
					OnStackCountChangedDelegate.Broadcast(InAbilityTag, StackCount);
				}
			);
			StackableAbilityManager->OnStackTimerStarted.BindLambda(
				[this](FGameplayTag InAbilityTag,float RechargeTime)
				{
					OnStackTimerStartedDelegate.Broadcast(InAbilityTag, RechargeTime);
				}
			);

			// 특별한 사용 타입이 하나라도 있으면 이 분기 안으로 들어갑니다.
			// 현재는 스택형밖에 없습니다.
			if (UsableTypeInfo.HasAnyTrue())
			{
				OnAbilityUsableTypeDelegate.Broadcast(AbilityUIInfo.AbilityTag, UsableTypeInfo);
				if (UsableTypeInfo.bIsStackable)
				{
					// Component의 충전 로직 첫 시작이 콜백 함수 바인드보다 먼저 이루어지기 때문에, 여기서 정보를 가져와 한 번 Broadcast해줍니다.
					if (const FAbilityStackItem* Item = StackableAbilityManager->FindItem(AbilityUIInfo.AbilityTag))
					{
						OnStackCountChangedDelegate.Broadcast(AbilityUIInfo.AbilityTag, Item->CurrentStack);
						OnStackTimerStartedDelegate.Broadcast(AbilityUIInfo.AbilityTag, Item->RechargeTime);
					}
				}
			}
		}
	}
}

void UOverlayWidgetController::OnXPChanged(const int32 InXP) const
{
	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	const ULevelUpInfo* LevelUpInfo = AuraPlayerState->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("LevelUpInfo를 찾을 수 없습니다. 블루프린트 에디터에서 할당해주세요."))

	const int32 Level = LevelUpInfo->FindLevelForXP(InXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		// LevelUpInfo를 토대로 XP Bar Percent를 계산합니다.
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		const int32 DeltaLevelUpRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = InXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelUpRequirement);
		OnXPBarPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

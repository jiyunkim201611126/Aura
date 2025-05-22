#include "AuraAbilitySystemComponent.h"

#include "Aura/Manager/AuraGameplayTags.h"
#include "Abilities/AuraGameplayAbility.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ThisClass::EffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		// 게임 시작 시 적용되는 Ability들
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			// Ability 적용을 기록하기 위해 태그 추가
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			// Ability 적용
			GiveAbility(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// 현재 적용 중인 Ability 가져오기
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 적용 중인 게 확실한지 Tag로 검증
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// Pressed 이벤트도 발생시킴
			AbilitySpecInputPressed(AbilitySpec);
			// Held 이벤트에 바인드된 Ability가 적용 중이지 않은 상태라면 적용 
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
                                                const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	// Widget Controller에게 Tag를 가진 Effect가 Apply되었음을 알림
	EffectAssetTags.Broadcast(TagContainer);
}

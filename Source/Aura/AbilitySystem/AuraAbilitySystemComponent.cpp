#include "AuraAbilitySystemComponent.h"

#include "Aura/Manager/AuraGameplayTags.h"
#include "Abilities/AuraGameplayAbility.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ThisClass::ClientEffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			// 어떤 Input에 의해 사용될 Ability인지 기록
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			// Ability 장착
			GiveAbility(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// 현재 장착 중인 Ability 가져오기
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 장착 중인 Ability 중 Tag가 일치하는 Ability가 있는지 탐색
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// Pressed 이벤트도 발생시킴
			AbilitySpecInputPressed(AbilitySpec);
			// Ability가 이미 작동 중이라면 더 작동시키지 않음
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

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	// Widget Controller에게 Tag를 가진 Effect가 Apply되었음을 알림
	EffectAssetTags.Broadcast(TagContainer);
}
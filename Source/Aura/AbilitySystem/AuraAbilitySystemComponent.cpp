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
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->StartupInputTag);
			// Ability 장착
			GiveAbility(AbilitySpec);
			// 부여된 Ability를 HUD에 스킬 아이콘으로 표시하기 위해 델리게이트를 Broadcast합니다.
			// 해당 함수는 서버에서만 호출되기 때문에, 클라이언트에서 OnRep 함수로 따로 Broadcast해줍니다.
			AbilitiesGivenDelegate.Broadcast(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		// PassiveAbility이므로, 부여와 동시에 발동합니다.
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::AddAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		GiveAbility(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	TSet<FGameplayAbilitySpecHandle> NewHandles;
	NewHandles.Reserve(GetActivatableAbilities().Num());

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.Handle.IsValid())
		{
			continue;
		}

		NewHandles.Add(AbilitySpec.Handle);

		// 이전에 갖고 있지 않던 Ability만 위젯에 알려줍니다.
		if (!CachedAbilityHandles.Contains(AbilitySpec.Handle))
		{
			AbilitiesGivenDelegate.Broadcast(AbilitySpec);
		}
	}

	// 추후 Ability가 제거되는 로직이 추가되면 여기에 로직을 작성해 위젯이 알 수 있도록 해줍니다.

	CachedAbilityHandles = MoveTemp(NewHandles);
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// 현재 장착 중인 Ability 가져오기
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 장착 중인 Ability 중 Tag가 일치하는 Ability가 있는지 탐색
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
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
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	// 매개변수로 들어온 AbilitySpec의 Abilities 태그를 찾아 반환합니다.
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->GetAssetTags())
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	// 매개변수로 들어온 AbilitySpec의 InputTag를 찾아 반환합니다.
	for (FGameplayTag Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	// Widget Controller에게 Tag를 가진 Effect가 Apply되었음을 알림
	EffectAssetTags.Broadcast(TagContainer);
}

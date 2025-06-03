#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer&);

UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// InitAbilityActorInfo(OwnerActor와 AvatarActor를 할당해주는 함수)가 호출된 직후 호출되는 함수
	void AbilityActorInfoSet();

	// Widget Controller가 바인드할 델리게이트
	FEffectAssetTags EffectAssetTags;

	// 키 입력으로 발동하는 Ability를 장착하는 함수
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);

	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

protected:
	// OnGameplayEffectAppliedDelegateToSelf에 붙이는 함수, 해당 델리게이트는 Server에서만 호출하기 때문에 RPC를 바인드해 클라이언트도 메시지 표시
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);
};

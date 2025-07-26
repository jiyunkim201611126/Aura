#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer&);
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitiesGiven, const FGameplayAbilitySpec&);

UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// InitAbilityActorInfo(OwnerActor와 AvatarActor를 할당해주는 함수)가 호출된 직후 호출되는 함수
	void AbilityActorInfoSet();

	// Widget Controller가 바인드할 델리게이트
	FEffectAssetTags EffectAssetTags;
	FAbilitiesGiven AbilitiesGivenDelegate;

	// 키 입력에 따라 발동하는 Ability를 장착하는 플레이어 캐릭터용 함수입니다. UAuraGameplayAbility를 사용합니다.
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	// 키 입력과 관계 없는 Ability를 장착하는 범용 함수입니다. UGameplayAbility를 사용합니다.
	void AddAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);
	
	// Ability가 부여될 때 클라이언트에서 호출되는 함수입니다.
	// AddCharacterAbilities는 서버에서만 호출되기 때문에, 클라이언트에서 따로 호출해줘야 합니다.
	virtual void OnRep_ActivateAbilities() override;

	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

protected:
	// OnGameplayEffectAppliedDelegateToSelf에 붙이는 함수, 해당 델리게이트는 Server에서만 호출하기 때문에 RPC를 바인드해 클라이언트도 메시지 표시
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);
};

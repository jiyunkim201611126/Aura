#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

class AStackableAbilityManager;
class UAbilityInfo;

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChanged, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, const int32 /*AbilityLevel*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, const FGameplayTag& /*InputTag*/, const FGameplayTag& /*PreviousInputTag*/)
DECLARE_MULTICAST_DELEGATE_OneParam(FDeactivatePassiveAbility, const FGameplayTag& /*AbilityTag*/);

UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// InitAbilityActorInfo(OwnerActor와 AvatarActor를 할당해주는 함수)가 호출된 직후 호출되는 함수
	void AbilityActorInfoSet();

	//~ Begin Object Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End Object Interface

	// 키 입력에 따라 발동하는 Ability를 장착하는 플레이어 캐릭터용 함수입니다. UAuraGameplayAbility를 사용합니다.
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	// 키 입력과 관계 없는 PassiveAbility를 장착하는 범용 함수입니다. UAuraGameplayAbility를 사용합니다.
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	// 키 입력과 관계 없는 Ability를 장착하는 범용 함수입니다. UGameplayAbility를 사용합니다.
	void AddAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);
	
	// 부여된 Ability에 변경 사항이 있는 경우 클라이언트에서 호출되는 함수입니다.
	// AddCharacterAbilities는 서버에서만 호출되기 때문에, 클라이언트에서 따로 호출해줘야 합니다.
	virtual void OnRep_ActivateAbilities() override;

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	FGameplayTag GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag);
	void ExtractTagsFromSpec(const FGameplayAbilitySpec& AbilitySpec, FGameplayTag& OutInputTag, FGameplayTag& OutStatusTag) const;

	FGameplayAbilitySpec* GetGivenAbilitySpecFromAbilityTag(const FGameplayTag& AbilityTag);

	bool GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FText& OutDescription, FText& OutNextLevelDescription, UAbilityInfo* AbilityInfo);

	void UpgradeAttribute(const FGameplayTag& AttributeTag);

	UFUNCTION(Server, Reliable)
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);

	void UpdateAbilityStatuses(int32 Level);

	UFUNCTION(Server, Reliable)
	void ServerSpendSpellPoint(const FGameplayTag& AbilityTag);

	UFUNCTION(Server, Reliable)
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag);
	
	template <class T>
	T* FindAbilityManager();
	template <class T>
	T* FindOrAddAbilityManager();

private:
	// OnGameplayEffectAppliedDelegateToSelf에 붙이는 함수, 해당 델리게이트는 Server에서만 호출하기 때문에 RPC를 바인드해 클라이언트도 메시지 표시
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);

	// 장착이 해제되면 해당 Ability의 InputTag를 제거하는 함수
	void ClearInputTag(FGameplayAbilitySpec* Spec);
	void ClearAbilitiesOfInputTag(const FGameplayAbilitySpec* AbilitySpec, const FGameplayTag& InputTag);
	bool AbilityHasInputTag(FGameplayAbilitySpec* Spec, const FGameplayTag& InputTag) const;
	
public:
	// Widget Controller가 바인드할 델리게이트
	FEffectAssetTags EffectAssetTags;
	FAbilityStatusChanged OnAbilityStatusOrLevelChangedDelegate;
	FAbilityEquipped OnAbilityEquipped;
	FDeactivatePassiveAbility OnDeactivatePassiveAbility;

private:
	// OnRep_ActivateAbilities가 호출되기 전, 클라이언트에서 캐싱해두고 있는 Ability 부여 정보입니다.
	// 갖고 있던 것과 비교해서 달라졌을 경우만 위젯에 알려줍니다.
	TMap<FGameplayAbilitySpecHandle, FGameplayAbilitySpec> CachedAbilities;

	// Ability Manager 배열입니다.
	// 런타임 중 드물게 변하는 배열이며, Owner 클라이언트에게만 복제되기 때문에 FastArray를 굳이 사용하지 않았습니다.
	// 현재는 Stackable Ability Manager 클래스뿐이며, 보유 Ability 중 하나라도 Stackable Ability라면 Manager 객체를 런타임 중에 할당받습니다.
	UPROPERTY(Replicated)
	TArray<TObjectPtr<AActor>> AdditionalCostManagers;
};

template<typename T>
T* UAuraAbilitySystemComponent::FindAbilityManager()
{
	for (AActor* Manager : AdditionalCostManagers)
	{
		if (T* Typed = Cast<T>(Manager))
		{
			return Typed;
		}
	}
	return nullptr;
}

template <class T>
T* UAuraAbilitySystemComponent::FindOrAddAbilityManager()
{
	if (T* Existing = FindAbilityManager<T>())
	{
		return Existing;
	}

	if (!IsOwnerActorAuthoritative())
	{
		// 클라이언트인 경우 생성할 권한이 없으므로 반환합니다.
		return nullptr;
	}
	
	// Manager 액터 생성 후 PlayerController에게 붙여 Owner 클라이언트에게만 Replicate됩니다.
	// AI인 경우에도 해당 객체가 필요하므로 주의합니다.
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (APlayerController* PlayerController = AbilityActorInfo->PlayerController.Get())
	{
		Params.Owner = PlayerController;
	}

	AActor* Avatar = GetAvatarActor();
		
	T* NewManager = GetWorld()->SpawnActor<T>(T::StaticClass(),
		Avatar ? Avatar->GetActorLocation() : FVector::ZeroVector,
		Avatar ? Avatar->GetActorRotation() : FRotator::ZeroRotator,
		Params);

	if (NewManager && Avatar && Avatar->GetRootComponent())
	{
		NewManager->AttachToComponent(Avatar->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		AdditionalCostManagers.Add(NewManager);
	}
	
	return NewManager;
}

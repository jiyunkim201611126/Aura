#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityEffectPolicy/AbilityEffectPolicy.h"
#include "AuraGameplayAbility.generated.h"

class UAuraAbilitySystemComponent;
class UAbilityAdditionalCost;

USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SocketName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ImpactSoundTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ImpactNiagaraTag;
};

UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// CombatTarget을 향해 몸을 돌리는 Enemy 전용 함수
	UFUNCTION(BlueprintCallable)
	void UpdateFacingToCombatTarget() const;

	UFUNCTION(BlueprintCallable)
	void ApplyAllEffect(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent)
	FText GetDescription(const int32 Level);
	static FText GetLockedDescription(const int32 Level);

protected:
	// 매개변수로 들어온 AbilityEffectPolicy 클래스가 갖고 있는 GameplayEffectContextHandle을 가져오는 함수입니다.
	// 반드시 Ability가 소유하고 있는 EffectPolicy만 사용해야 합니다.
	// 블루프린트에선 템플릿 함수를 지원하지 않기 때문에 Class를 매개변수로 받아 비슷한 동작을 하도록 구현합니다.
	UFUNCTION(BlueprintPure)
	FGameplayEffectContextHandle GetContextHandle(TSubclassOf<UAbilityEffectPolicy> PolicyClass) const;
	
	UFUNCTION(BlueprintPure)
	float GetManaCost(const int32 InLevel) const;
	UFUNCTION(BlueprintPure)
	float GetCooldown(const int32 InLevel) const;
	UFUNCTION(BlueprintPure)
	FText GetDamageTexts(const int32 InLevel) const;

	template<typename T>
	T* GetEffectPoliciesOfClass() const
	{
		// T가 UAbilityEffectPolicy를 상속받지 않는 경우 오류를 발생시키는 구문입니다.
		static_assert(TIsDerivedFrom<T, UAbilityEffectPolicy>::IsDerived, "T는 반드시 UAbilityEffectPolicy를 상속받아야 합니다.");

		for (UAbilityEffectPolicy* Policy : EffectPolicies)
		{
			if (Policy && Policy->IsA<T>())
			{
				return Cast<T>(Policy);
			}
		}

		return nullptr;
	}

private:
	// TaggedMontages 중 랜덤하게 하나 가져오는 함수입니다.
	UFUNCTION(BlueprintCallable, Category = "Montage")
	FTaggedMontage GetRandomMontage();

public:
	// 식별자 역할의 Ability의 전용 태그입니다.
	// 플레이어 캐릭터가 사용하는 Ability는 반드시 Tag 하나와 1:1 대응합니다.
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag AbilityTag;
	
	// 플레이어의 캐릭터만 사용하는 태그입니다.
	// Input과 관련된 태그들은 AuraInputConfig를 통해 InputAction과 매핑되어 있습니다.
	// AuraInputComponent를 통해 InputAction에 바인드된 함수는 호출 시 자동으로 연결된 태그가 매개변수로 들어갑니다.
	// 해당 매개변수로 사용자가 가진 Ability 중 태그가 일치하는 Ability를 가져와 호출하는 로직입니다.
	// 런타임 중 장착하는 Ability는 InputTag가 DynamicAbilityTags로 들어가며, 이 변수는 시작과 동시에 장착하는 Ability만 사용합니다.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;

protected:
	// 애니메이션 몽타주 및 각종 필요 변수를 한 번 래핑한 구조체의 배열입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TArray<FTaggedMontage> TaggedMontages;
	
	UPROPERTY(EditDefaultsOnly, Category = "Description")
	FString DescriptionKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Effect")
	TArray<TObjectPtr<UAbilityEffectPolicy>> EffectPolicies;

protected:
	// 이 아래로는 스택형 스킬을 구현하기 위한 구문입니다.
	// 아래 포인터 배열이 초기화되어선 안 되므로 반드시 Instanced per Actor로 설정해줍니다.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "AdditionalCost")
	TArray<TObjectPtr<UAbilityAdditionalCost>> AdditionalCosts;

public:
	void RegisterAbilityToAdditionalCostManagers(UAuraAbilitySystemComponent* ASC);
	void UnregisterAbilityFromAdditionalCostManagers(UAuraAbilitySystemComponent* ASC);
	
protected:
	//~ Begin UGameplayAbility Interface
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End UGameplayAbility Interface
	
	// AbilityTag로 할당한 Ability 전용 태그가 AssetTag(AbilityTags)에도 자동으로 추가되도록 해주는 함수들입니다.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
	void SyncAbilityTagToAssetTags();
#endif
};

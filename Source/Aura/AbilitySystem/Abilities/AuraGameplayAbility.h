#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

class UAuraAbilitySystemComponent;
class UAbilityUsableType;

USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SocketTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ImpactSoundTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag NiagaraTag;
};

USTRUCT(BlueprintType)
struct FDebuffData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag DebuffType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffChance = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffDamage = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffDuration = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffFrequency = 0.f;
};

UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// CombatTarget을 향해 몸을 돌리는 Enemy 전용 함수
	UFUNCTION(BlueprintCallable)
	void UpdateFacingToCombatTarget() const;

	UFUNCTION(BlueprintNativeEvent)
	FText GetDescription(int32 Level);
	static FText GetLockedDescription(int32 Level);

	UFUNCTION(BlueprintCallable)
	TArray<FGameplayEffectSpecHandle> MakeDebuffSpecHandle();

	UFUNCTION(BlueprintCallable)
	void CauseDebuff(AActor* TargetActor, const TArray<FGameplayEffectSpecHandle>& DebuffSpecs);

protected:
	
	UFUNCTION(BlueprintCallable)
	float GetManaCost(int32 InLevel = 1) const;
	UFUNCTION(BlueprintCallable)
	float GetCooldown(int32 InLevel = 1) const;

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
	// Input과 관련된 태그들은 AuraInputConfig를 통해 InputAction과 이어져있습니다.
	// AuraInputComponent를 통해 InputAction에 바인드된 함수는 호출 시 자동으로 연결된 태그가 매개변수로 들어갑니다.
	// 해당 매개변수로 사용자가 가진 Ability 중 태그가 일치하는 Ability를 가져와 호출하는 로직입니다.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;

	// 디버프 부여 용도로 사용되는 변수입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	TArray<FDebuffData> DebuffData;

protected:
	// 애니메이션 몽타주 및 각종 필요 변수를 한 번 래핑한 구조체의 배열입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TArray<FTaggedMontage> TaggedMontages;
	
	UPROPERTY(EditDefaultsOnly, Category = "Description")
	FString DescriptionKey;
	
	FGameplayEffectContextHandle DebuffEffectContextHandle;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	TSubclassOf<UGameplayEffect> DebuffEffectClass;

protected:
	// 이 아래로는 스택형 스킬을 구현하기 위한 구문입니다.
	// 아래 포인터 배열이 초기화되어선 안 되므로 반드시 Instanced per Actor로 설정해줍니다. 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UsableType")
	TArray<TObjectPtr<UAbilityUsableType>> UsableTypes;

public:
	void RegisterAbilityToUsableTypeManagers(UAuraAbilitySystemComponent* ASC);
	void UnregisterAbilityFromUsableTypeManagers(UAuraAbilitySystemComponent* ASC);
protected:
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	// AbilityTag로 할당한 Ability 전용 태그가 AssetTag(AbilityTags)에도 자동으로 추가되도록 해주는 함수들입니다.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
	void SyncAbilityTagToAssetTags();
#endif
};

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

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

UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAuraGameplayAbility();
	/**
	 * 플레이어의 캐릭터만 사용하는 태그입니다.
	 * Input과 관련된 태그들은 AuraInputConfig를 통해 InputAction과 이어져있습니다.
	 * AuraInputComponent를 통해 InputAction에 바인드된 함수는 호출 시 자동으로 연결된 태그가 매개변수로 들어갑니다.
	 * 해당 매개변수로 사용자가 가진 Ability 중 태그가 일치하는 Ability를 가져와 호출하는 로직입니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;

	// CombatTarget을 향해 몸을 돌리는 Enemy 전용 함수
	UFUNCTION(BlueprintCallable)
	void UpdateFacingToCombatTarget() const;

protected:
	// 애니메이션 몽타주 배열
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TArray<FTaggedMontage> TaggedMontages;

private:
	UFUNCTION(BlueprintCallable, Category = "Montage")
	FTaggedMontage GetRandomMontage();

public:
	/**
	 * 이 아래로는 충전식 스킬을 구현하기 위한 구문들입니다.
	 * Ability 객체가 충전 로직을 담당하게 되므로, 꼭 Instanced per Actor로 설정해줍니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chargeable")
	bool bUseCharges = false;

	UPROPERTY(BlueprintReadOnly, Category = "Chargeable", meta = (EditCondition = "bUseCharges"))
	int32 CurrentCharges = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chargeable", meta = (EditCondition = "bUseCharges"))
	int32 MaxCharges = 3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chargeable", meta = (EditCondition = "bUseCharges"))
	float RechargeTime = 5.f;

protected:
	FTimerHandle RechargeTimerHandle;

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
	void StartRecharge();
	void StopRecharge();
	void Recharge();
};

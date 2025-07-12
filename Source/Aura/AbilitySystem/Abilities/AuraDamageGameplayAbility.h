#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayAbility.h"
#include "Aura/Interaction/CombatInterface.h"
#include "AuraDamageGameplayAbility.generated.h"

UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	// Target Actor의 ASC에 GameplayEffect를 적용하는 함수
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);

	UFUNCTION(BlueprintCallable)
	FGameplayEffectContextHandle GetContext();
	
	FGameplayEffectContextHandle EffectContextHandle;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 데미지 타입과 그 속성 데미지
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;

	// 피격당한 Actor를 Context에 담는 함수입니다. 반복문이 끝난 시점에서 호출합니다.
	UFUNCTION(BlueprintCallable)
	void SetTargetActorsToContext();

private:
	// 이 Ability에 의해 피격당한 Actor를 모아두는 배열입니다.
	TArray<TWeakObjectPtr<AActor>> TargetActors;
};

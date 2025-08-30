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
	// ~Ability Interface
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// ~End of Ability Function

	UFUNCTION(BlueprintCallable)
	TArray<FGameplayEffectSpecHandle> MakeDamageSpecHandle();
	
	// Target Actor의 ASC에 GameplayEffect를 적용하는 함수
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor, TArray<FGameplayEffectSpecHandle> DamageSpecs);

	UFUNCTION(BlueprintCallable)
	FGameplayEffectContextHandle GetContext();

protected:
	UFUNCTION(BlueprintPure)
	FText GetDamageTexts(int32 InLevel);

public:
	FGameplayEffectContextHandle DamageEffectContextHandle;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 데미지 타입과 그 속성 데미지
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DeathImpulseMagnitude = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float KnockbackChance = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float KnockbackForceMagnitude = 500.f;
};

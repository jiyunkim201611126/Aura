#pragma once

#include "CoreMinimal.h"
#include "AbilityEffectPolicy.h"
#include "GameplayEffectTypes.h"
#include "ScalableFloat.h"
#include "AbilityEffectPolicy_Damage.generated.h"

class UGameplayEffect;

UCLASS()
class AURA_API UAbilityEffectPolicy_Damage : public UAbilityEffectPolicy
{
	GENERATED_BODY()

public:
	virtual void EndAbility() override;
	
	virtual void ApplyAllEffect(UGameplayAbility* OwningAbility, AActor* TargetActor) override;

	TArray<FGameplayEffectSpecHandle> MakeDamageSpecHandle(const UGameplayAbility* OwningAbility);
	void CauseDamage(const UGameplayAbility* OwningAbility, AActor* TargetActor, const TArray<FGameplayEffectSpecHandle>& DamageSpecs);
	FText GetDamageTexts(int32 InLevel);
	
	FGameplayEffectContextHandle DamageEffectContextHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DeathImpulseMagnitude = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float KnockbackChance = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float KnockbackForceMagnitude = 100.f;

protected:
	// 데미지 타입과 그 속성 데미지
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;
};

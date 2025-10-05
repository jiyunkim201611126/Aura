#pragma once

#include "CoreMinimal.h"
#include "AbilityEffectPolicy_Damage.h"
#include "AbilityEffectPolicy_RadialDamage.generated.h"

UCLASS()
class AURA_API UAbilityEffectPolicy_RadialDamage : public UAbilityEffectPolicy_Damage
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor) override;

	void SetRadialOriginLocation(const FVector& NewOriginLocation);
	TArray<FGameplayEffectSpecHandle> MakeDamageSpecHandleWithRadial(const UGameplayAbility* OwningAbility, const AActor* TargetActor);

private:
	float CalculateFalloffRatio(const float TargetDist) const;

public:
	// 최대 데미지로 적용될 수 있는 최대 거리입니다.
	UPROPERTY(EditDefaultsOnly, Category = "RadialDamage")
	float RadialDamageInnerRadius = 0.f;

	// 최소 데미지로 적용되기 시작하는 거리입니다.
	UPROPERTY(EditDefaultsOnly, Category = "RadialDamage")
	float RadialDamageOuterRadius = 0.f;

	// 최소 데미지 비율입니다.
	UPROPERTY(EditDefaultsOnly, Category = "RadialDamage")
	float MinDamageRatio = 0.2f;

	// Radial Origin Location 캐싱 용도의 Vector입니다.
	UPROPERTY(Transient)
	FVector RadialDamageOrigin = FVector::ZeroVector;
};

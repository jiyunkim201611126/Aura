#pragma once

#include "CoreMinimal.h"
#include "AbilityEffectPolicy_Damage.h"
#include "AbilityEffectPolicy_RadialFallOffDamage.generated.h"

UCLASS()
class AURA_API UAbilityEffectPolicy_RadialFallOffDamage : public UAbilityEffectPolicy_Damage
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor) override;

	void SetRadialOriginLocation(const FVector& NewOriginLocation);
	TArray<FGameplayEffectSpecHandle> MakeDamageSpecHandleWithRadial(const UGameplayAbility* OwningAbility, const AActor* TargetActor);
	
	virtual FText GetDamageTexts(int32 InLevel) override;

private:
	float CalculateFalloffRatio(const float TargetDist) const;

public:
	// 최대 데미지로 적용될 수 있는 최대 거리입니다.
	UPROPERTY(EditDefaultsOnly, Category = "RadialFallOffDamage")
	float RadialDamageInnerRadius = 0.f;

	// 최소 데미지로 적용되기 시작하는 거리입니다.
	UPROPERTY(EditDefaultsOnly, Category = "RadialFallOffDamage")
	float RadialDamageOuterRadius = 0.f;

	// 최소 데미지 비율입니다.
	UPROPERTY(EditDefaultsOnly, Category = "RadialFallOffDamage")
	float MinDamageRatio = 0.2f;

	// Radial Origin Location 캐싱 용도의 Vector입니다.
	UPROPERTY(Transient)
	FVector RadialDamageOrigin = FVector::ZeroVector;
};

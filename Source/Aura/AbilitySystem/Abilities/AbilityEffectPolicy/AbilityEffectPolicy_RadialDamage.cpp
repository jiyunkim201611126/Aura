#include "AbilityEffectPolicy_RadialDamage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

void UAbilityEffectPolicy_RadialDamage::ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor)
{
	checkf(RadialDamageOrigin != FVector::ZeroVector, TEXT("RadialDamage의 경우, Ability의 ApplyAllEffect 호출 전 SetRadialOrigin을 반드시 호출해야 합니다."));
	
	CauseDamage(OwningAbility, TargetActor, MakeDamageSpecHandleWithRadial(OwningAbility, TargetActor));
}

void UAbilityEffectPolicy_RadialDamage::SetRadialOriginLocation(const FVector& NewOriginLocation)
{
	RadialDamageOrigin = NewOriginLocation;
}

TArray<FGameplayEffectSpecHandle> UAbilityEffectPolicy_RadialDamage::MakeDamageSpecHandleWithRadial(const UGameplayAbility* OwningAbility, const AActor* TargetActor)
{
	const UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return TArray<FGameplayEffectSpecHandle>();
	}
	
	if (!EffectContextHandle.Get())
	{
		EffectContextHandle = ASC->MakeEffectContext();
	}

	// 데미지 원점으로부터 타겟까지 거리를 계산합니다.
	const float TargetDist = FVector::Dist(RadialDamageOrigin, TargetActor->GetActorLocation());

	// 데미지 감쇠율을 계산합니다.
	const float RadialDamageRatio = CalculateFalloffRatio(TargetDist);
	
	TArray<FGameplayEffectSpecHandle> DamageSpecs;
	for (TPair<FGameplayTag, FScalableFloat>& Pair : DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(OwningAbility->GetAbilityLevel());
		
		FGameplayEffectSpecHandle DamageSpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.f, EffectContextHandle);
		
		// 감쇠율이 적용된 데미지를 주입합니다.
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, Pair.Key, ScaledDamage * RadialDamageRatio);

		DamageSpecs.Add(DamageSpecHandle);
	}
	
	return DamageSpecs;
}

float UAbilityEffectPolicy_RadialDamage::CalculateFalloffRatio(const float TargetDist) const
{
	if (TargetDist < RadialDamageInnerRadius)
	{
		return 1.f;
	}
	
	const float FalloffRatio = 1.f - (TargetDist - RadialDamageInnerRadius) / (RadialDamageOuterRadius - RadialDamageInnerRadius);

	return FMath::Clamp(FalloffRatio, MinDamageRatio, 1.f);
}

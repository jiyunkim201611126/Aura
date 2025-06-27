#include "AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Kismet/KismetMathLibrary.h"

void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);
	for (TTuple<FGameplayTag, FScalableFloat> Pair : DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, Pair.Key, ScaledDamage);
	}
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}

void UAuraDamageGameplayAbility::UpdateFacingToCombatTarget() const
{
	UObject* SourceActor = GetAvatarActorFromActorInfo();
	const AActor* TargetActor = IEnemyInterface::Execute_GetCombatTarget(SourceActor);
	const FVector TargetLocation = TargetActor->GetActorLocation();
	ICombatInterface::Execute_UpdateFacingTarget(SourceActor, TargetLocation);
}

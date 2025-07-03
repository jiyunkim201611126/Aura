#include "AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "Aura/Interaction/CombatInterface.h"

void UAuraDamageGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	EffectContextHandle.Clear();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	// Context 생성 및 관련 Actor에 추가
	if (!EffectContextHandle.Get())
	{
		EffectContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	}

	TArray<TWeakObjectPtr<AActor>> Actors;
	Actors.Add(TWeakObjectPtr<AActor>(TargetActor));
	EffectContextHandle.AddActors(Actors);
	
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

FGameplayEffectContextHandle UAuraDamageGameplayAbility::GetContext()
{
	if (EffectContextHandle.Get())
	{
		return EffectContextHandle;
	}

	return FGameplayEffectContextHandle();
}

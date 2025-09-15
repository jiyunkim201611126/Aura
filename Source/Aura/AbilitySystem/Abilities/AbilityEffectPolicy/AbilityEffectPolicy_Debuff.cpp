#include "AbilityEffectPolicy_Debuff.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Manager/AuraGameplayTags.h"

void UAbilityEffectPolicy_Debuff::EndAbility()
{
	DebuffEffectContextHandle.Clear();
}

void UAbilityEffectPolicy_Debuff::ApplyAllEffect(UGameplayAbility* OwningAbility, AActor* TargetActor)
{
	CauseDebuff(OwningAbility, TargetActor, MakeDebuffSpecHandle(OwningAbility));
}

TArray<FGameplayEffectSpecHandle> UAbilityEffectPolicy_Debuff::MakeDebuffSpecHandle(const UGameplayAbility* OwningAbility)
{
	const UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return TArray<FGameplayEffectSpecHandle>();
	}
	
	if (!DebuffEffectContextHandle.Get())
	{
		DebuffEffectContextHandle = ASC->MakeEffectContext();
	}
	
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	TArray<FGameplayEffectSpecHandle> DebuffSpecs;
	for (const auto& Data : DebuffData)
	{
		// 디버프 부여 확률을 계산합니다.
		const bool bDebuff = FMath::FRandRange(0.f, 100.f) < Data.DebuffChance;
		if (!bDebuff)
		{
			continue;
		}
		
		FGameplayEffectSpecHandle DebuffSpecHandle = ASC->MakeOutgoingSpec(DebuffEffectClass, 1.f, DebuffEffectContextHandle);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DebuffSpecHandle, GameplayTags.Debuff_Damage, Data.DebuffDamage);
		UAbilitySystemBlueprintLibrary::SetDuration(DebuffSpecHandle, Data.DebuffDuration);
		DebuffSpecs.Add(DebuffSpecHandle);
	}

	return DebuffSpecs;
}

void UAbilityEffectPolicy_Debuff::CauseDebuff(const UGameplayAbility* OwningAbility, AActor* TargetActor, const TArray<FGameplayEffectSpecHandle>& DebuffSpecs)
{
	// 관련 Actor에 추가
	if (DebuffEffectContextHandle.IsValid())
	{
		TArray<TWeakObjectPtr<AActor>> TargetActors;
		TargetActors.Add(TargetActor);
		DebuffEffectContextHandle.AddActors(TargetActors);
	}
	
	for (auto& DebuffSpecHandle : DebuffSpecs)
	{
		if (TargetActor->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(TargetActor))
		{
			return;
		}
		
		if (UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo())
		{
			ASC->ApplyGameplayEffectSpecToTarget(*DebuffSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
		}
	}
}

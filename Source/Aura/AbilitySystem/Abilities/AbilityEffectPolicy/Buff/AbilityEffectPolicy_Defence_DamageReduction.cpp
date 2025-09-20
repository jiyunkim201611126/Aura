#include "AbilityEffectPolicy_Defence_DamageReduction.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbility.h"
#include "Aura/Manager/AuraGameplayTags.h"

void UAbilityEffectPolicy_Defence_DamageReduction::EndAbility()
{
	if (ActiveEffectHandle.IsValid())
	{
		if (AbilitySystemComponent.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectHandle, 1);
		}
	}
}

void UAbilityEffectPolicy_Defence_DamageReduction::ApplyAllEffect(UGameplayAbility* OwningAbility, AActor* TargetActor)
{
	if (EffectClass)
	{
		AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningAbility->GetAvatarActorFromActorInfo());
		if (AbilitySystemComponent.IsValid())
		{
			const FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
			const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, EffectContextHandle);
			EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FAuraGameplayTags::Get().Attributes_Defence_DamageReduction, DamageReductionMagnitude);
			ActiveEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		}
	}
}

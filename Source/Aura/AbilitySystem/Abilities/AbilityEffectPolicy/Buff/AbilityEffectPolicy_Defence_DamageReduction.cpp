#include "AbilityEffectPolicy_Defence_DamageReduction.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbility.h"
#include "Aura/Manager/AuraGameplayTags.h"

void UAbilityEffectPolicy_Defence_DamageReduction::ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor, const FEffectPolicyContext& EffectPolicyContext)
{
	if (EffectClass)
	{
		AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningAbility->GetAvatarActorFromActorInfo());
		if (AbilitySystemComponent.IsValid())
		{
			EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
			const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, EffectContextHandle);
			EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FAuraGameplayTags::Get().Attributes_Defence_DamageReduction, DamageReductionMagnitude);
			ActiveEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		}
	}
}

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

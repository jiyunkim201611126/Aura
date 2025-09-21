#include "AbilityEffectPolicy_Combat_AbsorbHP.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Manager/AuraGameplayTags.h"

void UAbilityEffectPolicy_Combat_AbsorbHP::ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor)
{
	if (EffectClass)
	{
		AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningAbility->GetAvatarActorFromActorInfo());
		if (AbilitySystemComponent.IsValid())
		{
			EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
			const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, EffectContextHandle);
			EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FAuraGameplayTags::Get().Attributes_Combat_AbsorbHealth, AbsorbHealthMagnitude);
			ActiveEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		}
	}
}

void UAbilityEffectPolicy_Combat_AbsorbHP::EndAbility()
{
	if (ActiveEffectHandle.IsValid())
	{
		if (AbilitySystemComponent.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectHandle, 1);
		}
	}
}

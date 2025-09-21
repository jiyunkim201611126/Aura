#include "AbilityEffectPolicy_Combat_AbsorbMana.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Manager/AuraGameplayTags.h"

void UAbilityEffectPolicy_Combat_AbsorbMana::ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor)
{
	if (EffectClass)
	{
		AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningAbility->GetAvatarActorFromActorInfo());
		if (AbilitySystemComponent.IsValid())
		{
			EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
			const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, EffectContextHandle);
			EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FAuraGameplayTags::Get().Attributes_Combat_AbsorbMana, AbsorbManaMagnitude);
			ActiveEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		}
	}
}

void UAbilityEffectPolicy_Combat_AbsorbMana::EndAbility()
{
	if (ActiveEffectHandle.IsValid())
	{
		if (AbilitySystemComponent.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectHandle, 1);
		}
	}
}

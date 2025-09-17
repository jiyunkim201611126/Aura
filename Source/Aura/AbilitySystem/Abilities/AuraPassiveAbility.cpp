#include "AuraPassiveAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"

void UAuraPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		ASC->OnDeactivatePassiveAbility.AddUObject(this, &ThisClass::ReceiveDeactivate);
	}
}

void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& InAbilityTag)
{
	if (GetAssetTags().HasTagExact(InAbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

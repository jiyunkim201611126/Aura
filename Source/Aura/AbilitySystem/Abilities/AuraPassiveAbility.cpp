#include "AuraPassiveAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/Niagara/AuraNiagaraComponent.h"
#include "GameFramework/Character.h"

void UAuraPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		ASC->OnDeactivatePassiveAbility.AddUObject(this, &ThisClass::ReceiveDeactivate);
	}

	if (ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		NiagaraComponent = NewObject<UAuraNiagaraComponent>(AvatarCharacter);
		if (NiagaraComponent)
		{
			NiagaraComponent->NiagaraTag = AbilityTag;
			NiagaraComponent->SetupAttachment(AvatarCharacter->GetMesh(), FName("RootSocket"));
			SetNiagaraComponentTransform();
			NiagaraComponent->RegisterComponent();
		}
	}
}

void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& InAbilityTag)
{
	if (GetAssetTags().HasTagExact(InAbilityTag))
	{
		if (NiagaraComponent)
		{
			NiagaraComponent->Deactivate();
			NiagaraComponent->DestroyComponent();
		}
		
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

#include "AbilityEffectPolicy.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

void UAbilityEffectPolicy::MakeEffectContextHandle(const UGameplayAbility* OwningAbility)
{
	// EffectContext를 생성 및 할당합니다.
	// MakeEffectContext 함수는 자동으로 OwnerActor를 Instigator로, AvatarActor를 EffectCauser로 할당합니다.
	if (!EffectContextHandle.Get())
	{
		if (const UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo())
		{
			EffectContextHandle = ASC->MakeEffectContext();
			EffectContextHandle.SetAbility(OwningAbility);
		}
	}
}

FGameplayEffectContextHandle UAbilityEffectPolicy::GetEffectContextHandle() const
{
	return EffectContextHandle;
}

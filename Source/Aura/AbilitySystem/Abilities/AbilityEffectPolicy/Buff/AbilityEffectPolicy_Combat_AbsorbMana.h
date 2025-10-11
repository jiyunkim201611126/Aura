#pragma once

#include "CoreMinimal.h"
#include "Aura/AbilitySystem/Abilities/AbilityEffectPolicy/AbilityEffectPolicy.h"
#include "AbilityEffectPolicy_Combat_AbsorbMana.generated.h"

UCLASS(Blueprintable)
class AURA_API UAbilityEffectPolicy_Combat_AbsorbMana : public UAbilityEffectPolicy
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor, const FEffectPolicyContext& EffectPolicyContext) override;
	virtual void EndAbility() override;

protected:
	UPROPERTY()
	FActiveGameplayEffectHandle ActiveEffectHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AbsorbManaMagnitude;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};


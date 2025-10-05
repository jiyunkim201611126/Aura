#pragma once

#include "CoreMinimal.h"
#include "Aura/AbilitySystem/Abilities/AbilityEffectPolicy/AbilityEffectPolicy.h"
#include "AbilityEffectPolicy_Combat_AbsorbHealth.generated.h"

UCLASS(Blueprintable)
class AURA_API UAbilityEffectPolicy_Combat_AbsorbHealth : public UAbilityEffectPolicy
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor) override;
	virtual void EndAbility() override;

protected:
	UPROPERTY()
	FActiveGameplayEffectHandle ActiveEffectHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AbsorbHealthMagnitude;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};

#pragma once

#include "CoreMinimal.h"
#include "AuraDamageGameplayAbility.h"
#include "Aura/Interaction/CombatInterface.h"
#include "AuraMeleeAttack.generated.h"

UCLASS()
class AURA_API UAuraMeleeAttack : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	FTaggedMontage GetRandomAttackMontage();
};

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GC_NiagaraWithLocationArray.generated.h"

UCLASS()
class AURA_API UGC_NiagaraWithLocationArray : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

	//~ Begin GameplayCueNotify_Static Interface
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	//~ End GameplayCueNotify_Static Interface
};

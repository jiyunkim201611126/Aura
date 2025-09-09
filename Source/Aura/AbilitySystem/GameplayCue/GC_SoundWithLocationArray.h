#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GC_SoundWithLocationArray.generated.h"

UCLASS()
class AURA_API UGC_SoundWithLocationArray : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};

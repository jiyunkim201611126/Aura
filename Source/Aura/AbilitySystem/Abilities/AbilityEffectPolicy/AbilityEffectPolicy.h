#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AbilityEffectPolicy.generated.h"

class UGameplayAbility;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class AURA_API UAbilityEffectPolicy : public UObject
{
	GENERATED_BODY()

public:
	virtual void EndAbility() PURE_VIRTUAL(...);
	
	virtual void ApplyAllEffect(UGameplayAbility* OwningAbility, AActor* TargetActor) PURE_VIRTUAL(...)
};

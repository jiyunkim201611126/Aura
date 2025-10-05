#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Aura/AbilitySystem/Abilities/AbilityEffectPolicy/AbilityEffectPolicy.h"
#include "AbilityEffectPolicy_Defence_DamageReduction.generated.h"

UCLASS(Blueprintable)
class AURA_API UAbilityEffectPolicy_Defence_DamageReduction : public UAbilityEffectPolicy
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor) override;
	virtual void EndAbility() override;

protected:
	// Remove Effect할 때 추적 용도로 사용됩니다.
	UPROPERTY()
	FActiveGameplayEffectHandle ActiveEffectHandle;

	// 데미지 경감 수치입니다.
	// 레벨에 따라 다른 수치를 주고 싶다면 FScalableFloat으로 변경, DT를 할당하면 됩니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageReductionMagnitude;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};

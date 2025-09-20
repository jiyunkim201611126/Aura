#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Aura/AbilitySystem/Abilities/AbilityEffectPolicy/AbilityEffectPolicy.h"
#include "AbilityEffectPolicy_Defence_DamageReduction.generated.h"

class UGameplayEffect;

UCLASS()
class AURA_API UAbilityEffectPolicy_Defence_DamageReduction : public UAbilityEffectPolicy
{
	GENERATED_BODY()

public:
	virtual void EndAbility() override;
	
	virtual void ApplyAllEffect(UGameplayAbility* OwningAbility, AActor* TargetActor) override;

protected:
	// 위 SpecHandle이 적용된 후 반환되는 구조체입니다.
	// Remove Effect할 때 추적 용도로 사용됩니다.
	UPROPERTY()
	FActiveGameplayEffectHandle ActiveEffectHandle;

	// 데미지 경감 수치입니다.
	// 레벨에 따라 다른 수치를 주고 싶다면 FScalableFloat으로 변경, DT를 할당하면 됩니다.
	UPROPERTY(EditDefaultsOnly)
	float DamageReductionMagnitude;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};

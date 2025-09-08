#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayAbility.h"
#include "AuraMeleeAttack.generated.h"

UCLASS()
class AURA_API UAuraMeleeAttack : public UAuraGameplayAbility
{
	GENERATED_BODY()

protected:
	// 공격 판정 시 구체 콜리전의 반지름입니다.
	// 실질적인 공격 범위를 의미합니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee")
	float AttackRadius = 100.f;
};

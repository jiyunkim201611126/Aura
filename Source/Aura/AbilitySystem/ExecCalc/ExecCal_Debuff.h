#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCal_Debuff.generated.h"

/**
 * 디버프 적용 여부를 계산하는 클래스입니다.
 */
UCLASS()
class AURA_API UExecCal_Debuff : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCal_Debuff();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

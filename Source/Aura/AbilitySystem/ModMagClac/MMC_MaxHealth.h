#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxHealth.generated.h"

/**
 * GameplayEffect가 적용될 때 Attribute를 통해 어떠한 계산 결과를 얻는 데에 사용하는 클래스입니다.
 * Modifier마다 별도의 MMC를 지정할 수 있습니다.
 * 블루프린트 클래스로도 구현할 수 있다는 장점이 있으나, 단 하나의 Attribute만 변경 가능하기 때문에 복잡한 로직은 구현 불가능합니다.
 * 단, 계산할 땐 여러 Attribute를 참조할 수 있습니다.
 * FGameplayEffectAttributeCaptureDefinition를 여러 개 선언한 뒤 각종 변수 초기화 후 RelevantAttributesToCapture에 등록하면 됩니다.
 * 예) 최대 체력의 20%만큼 회복, Health + Strength에 비례하는 데미지 등
 */

UCLASS()
class AURA_API UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_MaxHealth();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 어떤 Attribute를, 누구로부터, 언제 가져올지 정의하는 구조체
	FGameplayEffectAttributeCaptureDefinition VigorDef;
};

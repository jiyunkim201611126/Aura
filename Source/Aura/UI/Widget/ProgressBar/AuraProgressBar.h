#pragma once

#include "CoreMinimal.h"
#include "Aura/UI/Widget/AuraUserWidget.h"
#include "AuraProgressBar.generated.h"

class UProgressBar;

/**
 * ProgressBar 2개로 구현되는 프로젝트 기본 ProgressBar입니다.
 * Front는 값이 바로 변경되며, Ghost는 타이머를 통해 Front를 뒤따라갑니다.
 * 3개의 함수를 통해 Ghost가 조정되며, 블루프린트에서 오버라이드해 사용하는 것도 가능합니다.
 * 단, Size 조정이나 FillImage 등은 블루프린트에서 PreConstruct를 통해 초기화합니다.
 */

UCLASS()
class AURA_API UAuraProgressBar : public UAuraUserWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetBarPercent(const float Value, const float MaxValue);

	UFUNCTION(BlueprintNativeEvent)
	void BarPercentSet();

	UFUNCTION(BlueprintNativeEvent)
	void InterpGhostBar();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget = "true"))
	TObjectPtr<UProgressBar> ProgressBar_Front;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget = "true"))
	TObjectPtr<UProgressBar> ProgressBar_Ghost;

	FTimerHandle GhostPercentSetTimerHandle;

	float GhostPercentTarget = 0.5f;
	float GhostStartDelay = 1.f;
	float GhostInterpDelay = 0.02f;
	float GhostInterpSpeed = 3.f;
};

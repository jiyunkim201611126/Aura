#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

protected:
	/**
	 * WidgetController가 바인드되는 순간 호출하는 함수
	 * 주로 자신이 참조하고 있는 다른 위젯에 WidgetController를 할당하거나
	 * WidgetController의 델리게이트에 자신의 함수를 바인드하는 데에 사용
	 * 
	 * 예시:	OverlayWidget이 GlobeProgressBar의 SetWidgetController를 호출해 WidgetController를 할당,
	 *		GlobeProgressBar는 WidgetController를 OverlayWidgetController로 캐스트해 자신의 함수를 바인드
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};

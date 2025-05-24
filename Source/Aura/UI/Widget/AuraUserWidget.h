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

	// 전역함수를 통해 그때그때 가져와 사용할 수도 있으나, 웬만하면 멤버변수로 참조하고 있는 편이 낫습니다.
	// 명시적으로 의존성을 주입해 MVC 패턴의 안정성을 확보할 수 있습니다.
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

protected:
	/**
	 * WidgetController가 바인드되는 순간, 즉 SetWidgetController에서 호출하는 함수
	 * 주로 자신이 참조하고 있는 다른 위젯에 WidgetController를 할당하거나
	 * WidgetController의 델리게이트에 자신의 함수를 바인드하는 데에 사용
	 * 
	 * 예시:	OverlayWidget이 GlobeProgressBar의 SetWidgetController를 호출해 WidgetController를 할당,
	 *		GlobeProgressBar는 WidgetController를 OverlayWidgetController로 캐스트해 자신의 함수를 바인드
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};

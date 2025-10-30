#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadSlot.generated.h"

enum class ESaveSlotStatus : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/**
 * 해당 클래스는 LoadSlot 하나당 하나씩 할당되는 ViewModel 클래스입니다.
 * Field Notifies 단락의 멤버변수에 값 변화가 있는 경우, View에서 바인드한 콜백이 자동으로 호출됩니다.
 */
UCLASS()
class AURA_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void InitializeSlot();

public:
	// WidgetSwitcher에게 값을 전달하는 용도의 델리게이트입니다.
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;

	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;

	// SaveGame 클래스에서 사용하는 SlotName 용도의 변수입니다.
	UPROPERTY()
	FString LoadSlotName;

	// 현재 LoadSlot의 상태를 나타내는 변수입니다.
	UPROPERTY()
	ESaveSlotStatus LoadSlotStatus;

	UPROPERTY()
	FName PlayerStartTag;

	/**
	 * Field Notifies
	 * 아래 선언된 멤버변수들은 View 클래스인 블루프린트 위젯에서 참조할 수 있습니다.
	 * 참조되는 모습은 플러그인을 통해 UI로 확인할 수 있으며, 위젯의 어떤 요소가 영향을 받을지나 어떤 함수를 통해 반영할지 결정할 수 있습니다.
	 */
	
	void SetPlayerName(const FString& InPlayerName);
	FString GetPlayerName() const { return PlayerName; };
	
	void SetMapName(const FString& InMapName);
	FString GetMapName() const { return MapName; };

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = true))
	FString PlayerName;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = true))
	FString MapName;
};

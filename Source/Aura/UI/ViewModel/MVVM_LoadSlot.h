#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadSlot.generated.h"

enum class ESaveSlotStatus : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);

UCLASS()
class AURA_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void InitializeSlot();

public:
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;

	UPROPERTY()
	FString LoadSlotName;

	UPROPERTY()
	ESaveSlotStatus LoadSlotStatus;

	/** Field Notifies */

	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter)
	FString PlayerName;
	void SetPlayerName(const FString& InPlayerName);
	FString GetPlayerName() const { return PlayerName; };
};

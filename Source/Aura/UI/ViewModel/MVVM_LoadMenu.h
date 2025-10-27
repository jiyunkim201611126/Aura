#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadMenu.generated.h"

class UMVVM_LoadSlot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlotSelected);

/**
 * MVVM 플러그인의 핵심 클래스로, 해당 클래스는 LoadMenu의 동작을 총괄하는 ViewModel입니다.
 * 참조 관계는 View가 ViewModel을 참조하도록 되어있습니다.
 * 플레이어가 View를 조작하면 ViewModel의 함수가 호출되어, 적절한 델리게이트를 통해 콜백 함수를 호출합니다.
 * 해당 콜백은 View로 전달되어 화면에 표시됩니다.
 */
UCLASS()
class AURA_API UMVVM_LoadMenu : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void InitializeLoadSlot();

	/**
	 * LoadSlotViewModel 객체는 Slot의 개수만큼 생성되며, 순서대로 할당됩니다.
	 * 때문에 LoadSlotViewModel을 반환하는 함수를 따로 작성할 필요가 있습니다.
	 * 
	 * 반면, LoadMenuViewModel의 경우 블루프린트 위젯 클래스에서 HUD가 가진 객체를 가져와 캐스트 및 반환하는 함수를 사용합니다.
	 * 해당 함수는 플러그인의 기능을 통해 원하는 위젯에서 자동으로 호출되도록 구현할 수 있습니다.
	 */
	UFUNCTION(BlueprintPure)
	UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(int32 SlotIndex);

	// 빈 슬롯의 버튼을 클릭하면 호출되는 함수입니다.
	UFUNCTION(BlueprintCallable)
	void NewGameButtonPressed(int32 SlotIndex);

	// 신규 슬롯 할당을 위해 버튼을 클릭하면 호출되는 함수입니다.
	UFUNCTION(BlueprintCallable)
	void NewSlotButtonPressed(int32 SlotIndex, const FString& EnteredName);

	// 데이터가 할당된 슬롯의 버튼을 클릭하면 호출되는 함수입니다.
	UFUNCTION(BlueprintCallable)
	void SelectSlotButtonPressed(int32 SlotIndex);

	UFUNCTION(BlueprintCallable)
	void DeleteButtonPressed();

	void LoadData();

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadSlot> LoadSlotViewModelClass;

protected:
	UPROPERTY(BlueprintAssignable)
	FOnSlotSelected OnSlotSelected;

private:
	UPROPERTY()
	TArray<TObjectPtr<UMVVM_LoadSlot>> LoadSlotViewModels;

	UPROPERTY()
	int32 SelectedSlotIndex = INDEX_NONE;
};

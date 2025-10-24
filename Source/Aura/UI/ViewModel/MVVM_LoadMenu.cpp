#include "MVVM_LoadMenu.h"

#include "MVVM_LoadSlot.h"
#include "Aura/Game/AuraGameModeBase.h"
#include "Aura/Game/SaveGame/LoadMenuSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UMVVM_LoadMenu::InitializeLoadSlot()
{
	UMVVM_LoadSlot* NewLoadSlotViewModel_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	NewLoadSlotViewModel_0->LoadSlotName = FString("LoadSlot_0");
	LoadSlotViewModels.Add(NewLoadSlotViewModel_0);
	
	UMVVM_LoadSlot* NewLoadSlotViewModel_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	NewLoadSlotViewModel_1->LoadSlotName = FString("LoadSlot_1");
	LoadSlotViewModels.Add(NewLoadSlotViewModel_1);
	
	UMVVM_LoadSlot* NewLoadSlotViewModel_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	NewLoadSlotViewModel_2->LoadSlotName = FString("LoadSlot_2");
	LoadSlotViewModels.Add(NewLoadSlotViewModel_2);
}

UMVVM_LoadSlot* UMVVM_LoadMenu::GetLoadSlotViewModelByIndex(int32 SlotIndex)
{
	return LoadSlotViewModels[SlotIndex];
}

void UMVVM_LoadMenu::NewGameButtonPressed(int32 SlotIndex)
{
	// WidgetSwitcher의 1번째 인덱스에 해당하는 위젯(이름 입력란 위젯)을 표시합니다.
	LoadSlotViewModels[SlotIndex]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadMenu::NewSlotButtonPressed(int32 SlotIndex, const FString& EnteredName)
{
	// 입력된 이름을 저장합니다.
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));

	LoadSlotViewModels[SlotIndex]->SetPlayerName(EnteredName);
	LoadSlotViewModels[SlotIndex]->LoadSlotStatus = ESaveSlotStatus::Taken;

	AuraGameMode->SaveSlotData(LoadSlotViewModels[SlotIndex], SlotIndex);

	LoadSlotViewModels[SlotIndex]->InitializeSlot();
}

void UMVVM_LoadMenu::SelectSlotButtonPressed(int32 SlotIndex)
{
}

void UMVVM_LoadMenu::LoadData()
{
	// SlotName과 Index에 해당하는 SaveGame을 가져와 LoadSlot에 표시합니다.
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	int32 TempIndex = 0;
	for (auto LoadSlotViewModel : LoadSlotViewModels)
	{
		ULoadMenuSaveGame* SaveObject = AuraGameMode->GetSaveSlotData(LoadSlotViewModel->LoadSlotName, TempIndex++);
		
		const FString PlayerName = SaveObject->PlayerName;
		ESaveSlotStatus SaveSlotStatus = SaveObject->SaveSlotStatus;
		
		LoadSlotViewModel->SetPlayerName(PlayerName);
		LoadSlotViewModel->LoadSlotStatus = SaveSlotStatus;
		LoadSlotViewModel->InitializeSlot();
	}
}

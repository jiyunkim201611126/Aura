#include "MVVM_LoadMenu.h"

#include "MVVM_LoadSlot.h"
#include "Aura/Game/AuraGameModeBase.h"
#include "Aura/Game/AuraGameInstance.h"
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

	UMVVM_LoadSlot* LoadSlotViewModel = LoadSlotViewModels[SlotIndex];

	LoadSlotViewModel->SetMapName(AuraGameMode->DefaultMapName);
	LoadSlotViewModel->SetPlayerName(EnteredName);
	LoadSlotViewModel->PlayerStartTag = AuraGameMode->DefaultPlayerStartTag;
	LoadSlotViewModel->LoadSlotStatus = ESaveSlotStatus::Taken;

	AuraGameMode->SaveSlotData(LoadSlotViewModel, SlotIndex);

	LoadSlotViewModel->InitializeSlot();

	// GameInstance에 게임 저장 및 로드를 위해 필요한 값을 캐싱합니다.
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance());
	AuraGameInstance->LoadSlotName = LoadSlotViewModel->LoadSlotName;
	AuraGameInstance->LoadSlotIndex = SlotIndex;
	AuraGameInstance->PlayerStartTag = AuraGameMode->DefaultPlayerStartTag;
}

void UMVVM_LoadMenu::SelectSlotButtonPressed(int32 SlotIndex)
{
	OnSlotSelected.Broadcast();
	
	int32 TempIndex = 0;
	for (auto LoadSlotViewModel : LoadSlotViewModels)
	{
		// 선택한 Slot 외 다른 Slot들은 Button이 활성화되도록 알려줍니다.
		LoadSlotViewModel->EnableSelectSlotButton.Broadcast(SlotIndex != TempIndex++);
	}

	SelectedSlotIndex = SlotIndex;
}

void UMVVM_LoadMenu::DeleteButtonPressed()
{
	// 선택된 Slot이 있는지 확인합니다.
	if (SelectedSlotIndex != INDEX_NONE)
	{
		// 저장된 데이터를 삭제합니다.
		UMVVM_LoadSlot* SelectedSlotViewModel = LoadSlotViewModels[SelectedSlotIndex];
		AAuraGameModeBase::DeleteSlot(SelectedSlotViewModel->LoadSlotName, SelectedSlotIndex);

		// 슬롯을 초기화합니다.
		SelectedSlotViewModel->LoadSlotStatus = ESaveSlotStatus::Vacant;
		SelectedSlotViewModel->InitializeSlot();
		SelectedSlotViewModel->EnableSelectSlotButton.Broadcast(true);

		SelectedSlotIndex = INDEX_NONE;
	}
}

void UMVVM_LoadMenu::PlayButtonPressed()
{
	if (SelectedSlotIndex != INDEX_NONE)
	{
		AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
		UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance());
		AuraGameInstance->PlayerStartTag = LoadSlotViewModels[SelectedSlotIndex]->PlayerStartTag;
		AuraGameInstance->LoadSlotName = LoadSlotViewModels[SelectedSlotIndex]->LoadSlotName;
		AuraGameInstance->LoadSlotIndex = SelectedSlotIndex;
		
		AuraGameMode->TravelToMap(LoadSlotViewModels[SelectedSlotIndex]);
	}
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
		LoadSlotViewModel->SetMapName(SaveObject->MapName);
		LoadSlotViewModel->PlayerStartTag = SaveObject->PlayerStartTag;
		LoadSlotViewModel->LoadSlotStatus = SaveSlotStatus;
		LoadSlotViewModel->InitializeSlot();
	}
}

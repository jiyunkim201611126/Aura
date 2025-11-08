#include "MVVM_LoadMenu.h"

#include "MVVM_LoadSlot.h"
#include "Aura/Game/AuraGameInstance.h"
#include "Aura/Game/SaveGame/AuraSaveGame.h"
#include "Aura/Manager/SaveManagerSubsystem.h"
#include "Aura/Player/AuraPlayerController.h"

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
	USaveManagerSubsystem* SaveManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	
	UMVVM_LoadSlot* LoadSlotViewModel = LoadSlotViewModels[SlotIndex];

	// 새로운 슬롯을 생성 및 표시할 데이터를 할당합니다.
	// 할당된 데이터는 MVVM 플러그인 통해 View에 반영됩니다.
	LoadSlotViewModel->SetPlayerName(EnteredName);
	LoadSlotViewModel->SetPlayerLevel(1);
	LoadSlotViewModel->SetMapName(SaveManagerSubsystem->DefaultMapName);
	LoadSlotViewModel->PlayerStartTag = SaveManagerSubsystem->DefaultPlayerStartTag;
	LoadSlotViewModel->LoadSlotStatus = ESaveSlotStatus::Taken;

	SaveManagerSubsystem->SaveSlotData(LoadSlotViewModel, SlotIndex);

	LoadSlotViewModel->InitializeSlot();
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
		USaveManagerSubsystem* SaveManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
		UMVVM_LoadSlot* SelectedSlotViewModel = LoadSlotViewModels[SelectedSlotIndex];
		SaveManagerSubsystem->DeleteSlot(SelectedSlotViewModel->LoadSlotName, SelectedSlotIndex);

		// 슬롯을 초기화합니다.
		SelectedSlotViewModel->LoadSlotStatus = ESaveSlotStatus::Vacant;
		SelectedSlotViewModel->InitializeSlot();
		SelectedSlotViewModel->EnableSelectSlotButton.Broadcast(true);

		SelectedSlotIndex = INDEX_NONE;
	}
}

void UMVVM_LoadMenu::PlayButtonPressed(APlayerController* PlayerController)
{
	 // 해당 함수가 호출된 상황에서 RPC를 활용하기에 가장 적절한 객체가 PlayerController라고 판단되어, Widget으로부터 전달받아 활용합니다.
	if (PlayerController && SelectedSlotIndex != INDEX_NONE)
	{
		if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(PlayerController))
		{
			// 게임 저장 및 로드를 위해 필요한 값을 GameInstance에 캐싱합니다.
			// 캐싱된 정보는 레벨 이동 시 서버가 GameMode에서 접근해 활용합니다.
			UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetWorld()->GetGameInstance());
			UMVVM_LoadSlot* SelectedSlotViewModel = LoadSlotViewModels[SelectedSlotIndex];
			AuraGameInstance->PlayerStartTag = SelectedSlotViewModel->PlayerStartTag;
			AuraGameInstance->LoadSlotName = SelectedSlotViewModel->LoadSlotName;
			AuraGameInstance->LoadSlotIndex = SelectedSlotIndex;

			// 서버에 레벨 이동을 요청합니다.
			AuraPlayerController->ServerRequestTravel(SelectedSlotViewModel->GetMapName());
		}
	}
}

void UMVVM_LoadMenu::LoadData()
{
	// SlotName과 Index에 해당하는 SaveGame을 가져와 LoadSlot에 표시합니다.
	const USaveManagerSubsystem* SaveManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	int32 TempIndex = 0;
	for (const auto LoadSlotViewModel : LoadSlotViewModels)
	{
		const UAuraSaveGame* SaveData = SaveManagerSubsystem->GetSaveSlotData(LoadSlotViewModel->LoadSlotName, TempIndex++);

		const ESaveSlotStatus SaveSlotStatus = SaveData->SaveSlotStatus;
		const FString PlayerName = SaveData->PlayerName;
		
		LoadSlotViewModel->LoadSlotStatus = SaveSlotStatus;
		LoadSlotViewModel->SetPlayerName(PlayerName);
		LoadSlotViewModel->SetPlayerLevel(SaveData->PlayerLevel);
		LoadSlotViewModel->SetMapName(SaveData->MapName);
		LoadSlotViewModel->PlayerStartTag = SaveData->PlayerStartTag;
		
		LoadSlotViewModel->InitializeSlot();
	}
}

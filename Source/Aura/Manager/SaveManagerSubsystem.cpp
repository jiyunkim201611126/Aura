#include "SaveManagerSubsystem.h"

#include "Aura/Game/AuraGameInstance.h"
#include "Aura/Game/SaveGame/AuraSaveGame.h"
#include "Aura/UI/ViewModel/MVVM_LoadSlot.h"
#include "Kismet/GameplayStatics.h"

void USaveManagerSubsystem::SaveSlotData(const UMVVM_LoadSlot* LoadSlotViewModel, const int32 SlotIndex) const
{
	// 기존 저장된 데이터를 제거합니다.
	if (UGameplayStatics::DoesSaveGameExist(LoadSlotViewModel->LoadSlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlotViewModel->LoadSlotName, SlotIndex);
	}

	// 새로운 저장 데이터를 생성합니다.
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadMenuSaveGameClass);
	UAuraSaveGame* LoadMenuSaveGame = Cast<UAuraSaveGame>(SaveGameObject);
	LoadMenuSaveGame->SaveSlotStatus = LoadSlotViewModel->LoadSlotStatus;
	LoadMenuSaveGame->PlayerName = LoadSlotViewModel->GetPlayerName();
	LoadMenuSaveGame->MapName = LoadSlotViewModel->GetMapName();
	LoadMenuSaveGame->PlayerStartTag = LoadSlotViewModel->PlayerStartTag;

	UGameplayStatics::SaveGameToSlot(LoadMenuSaveGame, LoadSlotViewModel->LoadSlotName, SlotIndex);
}

UAuraSaveGame* USaveManagerSubsystem::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	// 기존에 저장 데이터가 없다면 생성, 있다면 해당 데이터를 반환합니다.
	USaveGame* SaveGameObject;
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}
	else
	{
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadMenuSaveGameClass);
	}
	UAuraSaveGame* LoadMenuSaveGame = Cast<UAuraSaveGame>(SaveGameObject);
	return LoadMenuSaveGame;
}

void USaveManagerSubsystem::DeleteSlot(const FString& SlotName, int32 SlotIndex) const
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
}

UAuraSaveGame* USaveManagerSubsystem::RetrieveInGameSaveData() const
{
	const UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());

	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;

	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}

void USaveManagerSubsystem::SaveInGameProgressData(UAuraSaveGame* SaveGameObject) const
{
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());

	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;

	AuraGameInstance->PlayerStartTag = SaveGameObject->PlayerStartTag;

	UGameplayStatics::SaveGameToSlot(SaveGameObject, InGameLoadSlotName, InGameLoadSlotIndex);
}

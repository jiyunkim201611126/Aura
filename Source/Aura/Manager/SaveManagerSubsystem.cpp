#include "SaveManagerSubsystem.h"

#include "Aura/Game/AuraGameInstance.h"
#include "Aura/Game/SaveGame/AuraSaveGame.h"
#include "Aura/Interaction/SavedActorInterface.h"
#include "Aura/UI/ViewModel/MVVM_LoadSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

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

void USaveManagerSubsystem::SaveWorldState(const UWorld* InWorld)
{
	FString WorldName = InWorld->GetMapName();
	WorldName.RemoveFromStart(InWorld->StreamingLevelsPrefix);

	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	check(AuraGameInstance);

	if (UAuraSaveGame* SaveData = GetSaveSlotData(AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex))
	{
		FSavedMap* SavedMap = SaveData->GetSavedMapWithMapName(WorldName);
		if (SavedMap == nullptr)
		{
			SavedMap->MapAssetName = WorldName;
			SaveData->SavedMaps.Add(*SavedMap);
		}

		SavedMap->SavedActors.Empty();

		for (TWeakObjectPtr<ISavedActorInterface> ActorToSave : ActorToSaveInCurrentLevel)
		{
			if (!ActorToSave.IsValid())
			{
				continue;
			}
			
			if (AActor* Actor = Cast<AActor>(ActorToSave.Get()))
			{
				FSavedActor SavedActor;
				SavedActor.ActorName = Actor->GetFName();
				SavedActor.Transform = Actor->GetTransform();

				FMemoryWriter MemoryWriter(SavedActor.Bytes);

				FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
				Archive.ArIsSaveGame = true;

				Actor->Serialize(Archive);

				SavedMap->SavedActors.AddUnique(SavedActor);
			}
		}

		for (FSavedMap& MapToReplace : SaveData->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = *SavedMap;
			}
		}

		UGameplayStatics::SaveGameToSlot(SaveData, AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex);
	}
}

void USaveManagerSubsystem::LoadWorldState(const UWorld* InWorld)
{
	FString WorldName = InWorld->GetMapName();
	WorldName.RemoveFromStart(InWorld->StreamingLevelsPrefix);

	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	check(AuraGameInstance);

	if (UGameplayStatics::DoesSaveGameExist(AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex))
	{
		UAuraSaveGame* SaveData = Cast<UAuraSaveGame>(UGameplayStatics::LoadGameFromSlot(AuraGameInstance->LoadSlotName, AuraGameInstance->LoadSlotIndex));
		if (SaveData == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("저장된 데이터를 불러오는 데에 실패했습니다."));
			return;
		}
		
		for (TWeakObjectPtr<ISavedActorInterface> ActorToSave : ActorToSaveInCurrentLevel)
		{
			if (!ActorToSave.IsValid())
			{
				continue;
			}

			if (AActor* Actor = Cast<AActor>(ActorToSave.Get()))
			{
				for (FSavedActor SavedActor : SaveData->GetSavedMapWithMapName(WorldName)->SavedActors)
				{
					if (SavedActor.ActorName == Actor->GetFName())
					{
						
					}
				}
			}
		}
	}
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

void USaveManagerSubsystem::AddActorToSave(ISavedActorInterface* ActorToSave)
{
	if (ActorToSave)
	{
		ActorToSaveInCurrentLevel.Add(ActorToSave);
	}
}

void USaveManagerSubsystem::RemoveActorToSave(ISavedActorInterface* ActorToSave)
{
	if (ActorToSave)
	{
		ActorToSaveInCurrentLevel.RemoveSingleSwap(ActorToSave);
	}
}

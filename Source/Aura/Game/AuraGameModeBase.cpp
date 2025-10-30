#include "AuraGameModeBase.h"

#include "Aura/UI/ViewModel/MVVM_LoadSlot.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGame/AuraGameInstance.h"
#include "SaveGame/LoadMenuSaveGame.h"

void AAuraGameModeBase::SaveSlotData(const UMVVM_LoadSlot* LoadSlotViewModel, const int32 SlotIndex) const
{
	// 기존 저장된 데이터를 제거합니다.
	if (UGameplayStatics::DoesSaveGameExist(LoadSlotViewModel->LoadSlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlotViewModel->LoadSlotName, SlotIndex);
	}

	// 새로운 저장 데이터를 생성합니다.
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadMenuSaveGameClass);
	ULoadMenuSaveGame* LoadMenuSaveGame = Cast<ULoadMenuSaveGame>(SaveGameObject);
	LoadMenuSaveGame->PlayerName = LoadSlotViewModel->GetPlayerName();
	LoadMenuSaveGame->MapName = LoadSlotViewModel->GetMapName();
	LoadMenuSaveGame->SaveSlotStatus = LoadSlotViewModel->LoadSlotStatus;

	UGameplayStatics::SaveGameToSlot(LoadMenuSaveGame, LoadSlotViewModel->LoadSlotName, SlotIndex);
}

ULoadMenuSaveGame* AAuraGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
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
	ULoadMenuSaveGame* LoadMenuSaveGame = Cast<ULoadMenuSaveGame>(SaveGameObject);
	return LoadMenuSaveGame;
}

void AAuraGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
}

void AAuraGameModeBase::TravelToMap(const UMVVM_LoadSlot* LoadSlotViewModel)
{
	const FString SlotName = LoadSlotViewModel->LoadSlotName;
	
	UGameplayStatics::OpenLevelBySoftObjectPtr(LoadSlotViewModel, Maps.FindChecked(LoadSlotViewModel->GetMapName()));
}

void AAuraGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	Maps.Add(DefaultMapName, DefaultMap);
}

AActor* AAuraGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);

	if (Actors.Num() > 0)
	{
		AActor* SelectedActor = Actors[0];
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				// PlayerStart의 Tag 중 GameInstance에 캐싱된 Tag와 일치하는 PlayerStart를 탐색합니다. 
				if (PlayerStart->PlayerStartTag == AuraGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}
	return nullptr;
}

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveManagerSubsystem.generated.h"

class UMVVM_LoadSlot;
class ULoadMenuSaveGame;
class USaveGame;

UCLASS(config = Game)
class AURA_API USaveManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void SaveSlotData(const UMVVM_LoadSlot* LoadSlotViewModel, const int32 SlotIndex) const;
	ULoadMenuSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;
	void DeleteSlot(const FString& SlotName, int32 SlotIndex) const;
	ULoadMenuSaveGame* RetrieveInGameSaveData() const;
	void SaveInGameProgressData(ULoadMenuSaveGame* SaveGameObject) const;

public:
	UPROPERTY()
	FString DefaultMapName = TEXT("First Dungeon");

	UPROPERTY()
	FName DefaultPlayerStartTag = TEXT("Dungeon1");

private:
	UPROPERTY(Config)
	TSubclassOf<USaveGame> LoadMenuSaveGameClass;
};

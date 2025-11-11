#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveManagerSubsystem.generated.h"

class ISavedActorInterface;
class UMVVM_LoadSlot;
class UAuraSaveGame;
class USaveGame;

/**
 * 저장 및 불러오기는 모두 서버의 로직상에서 이루어집니다.
 * 즉, 클라이언트는 저장된 데이터를 갖지 못 합니다.
 * 테스트 환경에서는 서버와 클라이언트가 같은 기기이므로 작동상에 문제는 없습니다.
 * 실제 멀티플레이 환경에선 클라이언트가 Widget에 저장된 데이터를 표시하지 못 하므로, 이를 RPC로 보내주거나 따로 저장하게 만들 필요가 있습니다.
 */
UCLASS(config = Game)
class AURA_API USaveManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void SaveSlotData(const UMVVM_LoadSlot* LoadSlotViewModel, const int32 SlotIndex) const;
	UAuraSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;
	void DeleteSlot(const FString& SlotName, int32 SlotIndex) const;
	UAuraSaveGame* RetrieveInGameSaveData() const;
	void SaveInGameProgressData(UAuraSaveGame* SaveGameObject) const;

	void AddActorToSave(ISavedActorInterface* ActorToSave);
	void RemoveActorToSave(ISavedActorInterface* ActorToSave);

	// 월드 내에 ISaveActorInterface를 상속받아 구현된 Actor의 정보를 직렬화해 저장 및 불러오는 함수입니다.
	void SaveWorldState(const UWorld* InWorld);
	void LoadWorldState(const UWorld* InWorld);

public:
	UPROPERTY()
	FString DefaultMapName = TEXT("First Dungeon");

	UPROPERTY()
	FName DefaultPlayerStartTag = TEXT("Dungeon1");

private:
	UPROPERTY(Config)
	TSubclassOf<USaveGame> LoadMenuSaveGameClass;

	// 레벨 내의 저장할 액터를 빠르게 추적하기 위해 캐싱해두는 변수입니다.
	UPROPERTY()
	TArray<TWeakObjectPtr<ISavedActorInterface>> ActorToSaveInCurrentLevel;
};

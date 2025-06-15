#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PawnManagerSubsystem.generated.h"

class APawn;

/**
 * 플레이어와 몬스터들의 Pawn을 등록하는 Subsystem입니다.
 * 주로 AI들이 플레이어 색적을 위해 사용합니다.
 */

UCLASS(BlueprintType)
class AURA_API UPawnManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<APawn>> PlayerPawns;

	UPROPERTY()
	TArray<TWeakObjectPtr<APawn>> AIPawns;

	UPROPERTY()
	TArray<APawn*> CachedPlayerPawns;
	UPROPERTY()
	TArray<APawn*> CachedAIPawns;

	bool bPlayerPawnsCacheDirty = true;
	bool bAIPawnsCacheDirty = true;

public:
	void RegisterPlayerPawn(APawn* InPawn);
	void UnregisterPlayerPawn(APawn* InPawn);

	UFUNCTION(BlueprintCallable)
	TArray<APawn*> GetAllPlayerPawns();
	
	void RegisterAIPawn(APawn* InPawn);
	void UnregisterAIPawn(APawn* InPawn);

	UFUNCTION(BlueprintCallable)
	TArray<APawn*> GetAllAIPawns();
};

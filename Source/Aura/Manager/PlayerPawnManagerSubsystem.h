#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerPawnManagerSubsystem.generated.h"

class APawn;

/**
 * 플레이어들의 Pawn을 등록하는 Subsystem입니다.
 * 주로 AI들이 플레이어 색적을 위해 사용합니다.
 */

UCLASS(BlueprintType)
class AURA_API UPlayerPawnManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<APawn*> PlayerPawns;

public:
	void RegisterPlayerPawn(APawn* InPawn);
	void UnregisterPlayerPawn(APawn* InPawn);

	UFUNCTION(BlueprintCallable)
	TArray<APawn*> GetAllPlayerPawns();
};

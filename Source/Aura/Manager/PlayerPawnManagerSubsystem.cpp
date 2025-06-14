#include "PlayerPawnManagerSubsystem.h"

void UPlayerPawnManagerSubsystem::RegisterPlayerPawn(APawn* InPawn)
{
	PlayerPawns.Add(InPawn);
}

void UPlayerPawnManagerSubsystem::UnregisterPlayerPawn(APawn* InPawn)
{
	PlayerPawns.Remove(InPawn);
}

TArray<APawn*> UPlayerPawnManagerSubsystem::GetAllPlayerPawns()
{
	return PlayerPawns;
}

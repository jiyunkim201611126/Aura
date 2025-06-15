#include "PawnManagerSubsystem.h"

void UPawnManagerSubsystem::RegisterPlayerPawn(APawn* InPawn)
{
	if (!PlayerPawns.Contains(InPawn))
	{
		PlayerPawns.Add(InPawn);
		bPlayerPawnsCacheDirty = true;
	}
}

void UPawnManagerSubsystem::UnregisterPlayerPawn(APawn* InPawn)
{
	PlayerPawns.RemoveSingleSwap(InPawn);
	bPlayerPawnsCacheDirty = true;
}

TArray<APawn*> UPawnManagerSubsystem::GetAllPlayerPawns()
{
	if (bPlayerPawnsCacheDirty)
	{
		CachedPlayerPawns.Reset();
		for (const TWeakObjectPtr<APawn>& PlayerPawn : PlayerPawns)
		{
			if (PlayerPawn.IsValid())
			{
				CachedPlayerPawns.Add(PlayerPawn.Get());
			}
		}
		bPlayerPawnsCacheDirty = false;
	}
	return CachedPlayerPawns;
}

void UPawnManagerSubsystem::RegisterAIPawn(APawn* InPawn)
{
	if (!AIPawns.Contains(InPawn))
	{
		AIPawns.Add(InPawn);
		bAIPawnsCacheDirty = true;
	}
}

void UPawnManagerSubsystem::UnregisterAIPawn(APawn* InPawn)
{
	AIPawns.RemoveSingleSwap(InPawn);
	bAIPawnsCacheDirty = true;
}

TArray<APawn*> UPawnManagerSubsystem::GetAllAIPawns()
{
	if (bAIPawnsCacheDirty)
	{
		CachedAIPawns.Reset();
		for (const TWeakObjectPtr<APawn>& AIPawn : CachedAIPawns)
		{
			if (AIPawn.IsValid())
			{
				CachedAIPawns.Add(AIPawn.Get());
			}
		}
		bAIPawnsCacheDirty = false;
	}
	return CachedAIPawns;
}

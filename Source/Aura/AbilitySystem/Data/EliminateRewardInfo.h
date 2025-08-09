#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Engine/DataAsset.h"
#include "EliminateRewardInfo.generated.h"

UENUM(BlueprintType)
enum class ECharacterRank : uint8
{
	None,
	Normal,
	Elite,
	Boss,

	EndCharacterRank
};

USTRUCT(BlueprintType)
struct FEliminateRewardDefaultInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Reward | XP")
	FScalableFloat XPReward = FScalableFloat();
};

UCLASS()
class AURA_API UEliminateRewardInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	FEliminateRewardDefaultInfo GetEliminateRewardInfoByRank(ECharacterRank Rank);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Eliminate Rewards")
	TMap<ECharacterRank, FEliminateRewardDefaultInfo> EliminateRewardInformation;
};

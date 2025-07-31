#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfo.generated.h"

USTRUCT(BlueprintType)
struct FAuraLevelUpInfo
{
	GENERATED_BODY()

	// 다음 레벨까지 필요한 경험치입니다.
	UPROPERTY(EditDefaultsOnly)
	int32 LevelUpRequirement = 0;

	// 레벨업 시 주어지는 Attribute Point입니다.
	UPROPERTY(EditDefaultsOnly)
	int32 AttributePointAward = 1;

	// 레벨업 시 주어지는 Spell Point입니다.
	UPROPERTY(EditDefaultsOnly)
	int32 SpellPointAward = 1;
};

UCLASS()
class AURA_API ULevelUpInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FAuraLevelUpInfo> LevelUpInformation;

	// XP 보유량에 따라 레벨이 몇인지 탐색하는 함수입니다.
	int32 FindLevelForXP(int32 XP) const;
};

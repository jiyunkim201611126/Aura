#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"

class UEliminateRewardInfo;
class UCharacterClassInfo;
class UAbilityInfo;

UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UEliminateRewardInfo> EliminateRewardInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UAbilityInfo> AbilityInfo;
};

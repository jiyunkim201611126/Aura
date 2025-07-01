#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SoundManagerSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FTaggedSoundRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SoundTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SoundAsset;
};

UCLASS(Config = Game)
class AURA_API USoundManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	USoundBase* GetSound(const FGameplayTag SoundTag);

private:
	UPROPERTY(Config)
	TSoftObjectPtr<UDataTable> TaggedSoundDataTable;
	
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> SoundInfos;
};

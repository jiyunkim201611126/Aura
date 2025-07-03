#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FXManagerSubsystem.generated.h"

class UNiagaraSystem;
struct FStreamableManager;

USTRUCT(BlueprintType)
struct FTaggedSoundRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SoundTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SoundAsset;
};

USTRUCT(BlueprintType)
struct FTaggedNiagaraRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag NiagaraTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> NiagaraAsset;
};

// 내부적으로 로딩 중인 에셋의 상태를 관리할 구조체
USTRUCT()
struct FSoundAsyncLoadRequest
{
	GENERATED_BODY()

	TOptional<TSharedPtr<FStreamableHandle>> StreamableHandle;

	UPROPERTY()
	TArray<FVector> LocationsToPlay;

	UPROPERTY()
	TArray<FRotator> RotationsToPlay;

	UPROPERTY()
	TArray<float> VolumeMultiplier;

	UPROPERTY()
	TArray<float> PitchMultiplier;

	// 필요하면 변수 추가
	
	FSoundAsyncLoadRequest()
	{
	}
};

USTRUCT()
struct FNiagaraAsyncLoadRequest
{
	GENERATED_BODY()

	TOptional<TSharedPtr<FStreamableHandle>> StreamableHandle;

	UPROPERTY()
	TArray<FVector> LocationsToPlay;

	UPROPERTY()
	TArray<FRotator> RotationsToPlay;

	UPROPERTY()
	TArray<FVector> ScalesToPlay;

	UPROPERTY()
	TArray<bool> bAutoDestroy;

	UPROPERTY()
	TArray<bool> bAutoActivate;

	// 필요하면 변수 추가
	
	FNiagaraAsyncLoadRequest()
	{
	}
};

UCLASS(Config = Game)
class AURA_API UFXManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "FX")
	void AsyncPlaySoundAtLocation(const FGameplayTag SoundTag, const FVector Location, const FRotator Rotation = FRotator::ZeroRotator, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f);
	void OnSoundAsyncLoadComplete(FSoftObjectPath LoadedAssetPath);

	UFUNCTION(BlueprintCallable, Category = "FX")
	void AsyncPlayNiagaraAtLocation(const FGameplayTag NiagaraTag, const FVector Location, const FRotator Rotation = FRotator::ZeroRotator, const FVector Scale = FVector(1.f), bool bAutoDestroy = true, bool bAutoActivate = true);
	void OnNiagaraAsyncLoadComplete(FSoftObjectPath LoadedAssetPath);

private:
	// DataTable에 매핑되어있는 Tag와 에셋들은 탐색 효율을 위해 TMap으로 재구성되므로, 메모리 효율을 위해 Soft로 선언합니다.
	UPROPERTY(Config)
	TSoftObjectPtr<UDataTable> SoundDataTablePath;
	
	UPROPERTY(Config)
	TSoftObjectPtr<UDataTable> NiagaraDataTablePath;

	UPROPERTY()
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> SoundInfos;
	
	UPROPERTY()
	TMap<FGameplayTag, TSoftObjectPtr<UNiagaraSystem>> NiagaraInfos;
	
	FStreamableManager* StreamableManager;
	
	// 현재 로딩 중인 SoftObjectPath와 그에 해당하는 로딩 정보 매핑
	UPROPERTY()
	TMap<FSoftObjectPath, FSoundAsyncLoadRequest> PendingSoundLoadRequests;
	
	UPROPERTY()
	TMap<FSoftObjectPath, FNiagaraAsyncLoadRequest> PendingNiagaraLoadRequests;

	// 멀티스레딩 안전성 확보를 위한 변수
	FCriticalSection PendingRequestsLock;
};

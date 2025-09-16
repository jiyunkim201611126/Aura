#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
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

USTRUCT()
struct FSoundAsyncPlayData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector LocationsToPlay;

	UPROPERTY()
	FRotator RotationsToPlay;

	UPROPERTY()
	float VolumeMultiplier;

	UPROPERTY()
	float PitchMultiplier;

	// 필요하면 변수 추가
	
	FSoundAsyncPlayData()
	{
	}
};

USTRUCT()
struct FSoundAsyncLoadRequest
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FSoundAsyncPlayData> PlayRequests;

	TArray<TFunction<void(USoundBase*)>> GetterCallbacks;
	
	FSoundAsyncLoadRequest()
	{
	}
};

USTRUCT()
struct FNiagaraAsyncSpawnData
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector Scale;

	UPROPERTY()
	bool bAutoDestroy = true;

	UPROPERTY()
	bool bAutoActivate = true;
};

USTRUCT()
struct FNiagaraAsyncLoadRequest
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FNiagaraAsyncSpawnData> SpawnRequests;

	TArray<TFunction<void(UNiagaraSystem*)>> GetterCallbacks;
	
	FNiagaraAsyncLoadRequest()
	{
	}
};

UCLASS(Config = Game)
class AURA_API UFXManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin Subsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End Subsystem Interface

	UFUNCTION(BlueprintCallable, Category = "FX")
	void AsyncPlaySoundAtLocation(const FGameplayTag& SoundTag, const FVector Location, const FRotator Rotation = FRotator::ZeroRotator, const float VolumeMultiplier = 1.f, const float PitchMultiplier = 1.f);
	void AsyncGetSound(const FGameplayTag& SoundTag, const TFunction<void(USoundBase*)>& OnLoadedCallback);
	void OnSoundAsyncLoadComplete(FSoftObjectPath LoadedAssetPath);

	UFUNCTION(BlueprintCallable, Category = "FX")
	void AsyncSpawnNiagaraAtLocation(const FGameplayTag& NiagaraTag, const FVector Location, const FRotator Rotation = FRotator::ZeroRotator, const FVector Scale = FVector(1.f), bool bAutoDestroy = true, bool bAutoActivate = true);
	void AsyncGetNiagara(const FGameplayTag& NiagaraTag, const TFunction<void(UNiagaraSystem*)>& OnLoadedCallback);
	void OnNiagaraAsyncLoadComplete(FSoftObjectPath LoadedAssetPath);

	// 동기 로드를 원하는 경우 사용하는 함수
	USoundBase* GetSound(const FGameplayTag& SoundTag) const;
	UNiagaraSystem* GetNiagara(const FGameplayTag& NiagaraTag) const;

private:
	// DataTable에 매핑되어있는 Tag와 에셋들은 탐색 효율을 위해 TMap으로 재구성되므로, 메모리 효율을 위해 Soft로 선언합니다.
	UPROPERTY(Config)
	TSoftObjectPtr<UDataTable> SoundDataTablePath;
	
	UPROPERTY(Config)
	TSoftObjectPtr<UDataTable> NiagaraDataTablePath;

	UPROPERTY()
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> SoundMap;
	
	UPROPERTY()
	TMap<FGameplayTag, TSoftObjectPtr<UNiagaraSystem>> NiagaraMap;
	
	FStreamableManager* StreamableManager;
	
	// 동일한 에셋에 대한 요청이 여러 개 동시에 들어올 경우, 로드가 끝날 때 모두 처리할 수 있도록 콜백 함수나 재생 관련 변수를 캐싱해 대기시키는 TMap입니다.
	UPROPERTY()
	TMap<FSoftObjectPath, FSoundAsyncLoadRequest> PendingSoundLoadRequests;
	
	UPROPERTY()
	TMap<FSoftObjectPath, FNiagaraAsyncLoadRequest> PendingNiagaraLoadRequests;

	// 멀티스레딩 안전성 확보를 위한 변수
	FCriticalSection PendingRequestsLock;
};

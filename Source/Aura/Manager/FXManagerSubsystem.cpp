#include "FXManagerSubsystem.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"

void UFXManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	StreamableManager = &UAssetManager::Get().GetStreamableManager();

	// 모든 FX를 동기 로드합니다.

	if (const UDataTable* SoundDataTable = SoundDataTablePath.LoadSynchronous())
	{
		const FString Context(TEXT("FXManagerSubsystem - Sound"));
		TArray<FTaggedSoundRow*> Rows;
		SoundDataTable->GetAllRows<FTaggedSoundRow>(Context, Rows);

		for (const FTaggedSoundRow* Row : Rows)
		{
			if (Row && Row->SoundTag.IsValid() && !Row->SoundAsset.IsNull())
			{
				SoundInfos.Add(Row->SoundTag, Row->SoundAsset);
			}
		}
	}

	if (const UDataTable* NiagaraDataTable = NiagaraDataTablePath.LoadSynchronous())
	{
		const FString Context(TEXT("FXManagerSubsystem - Niagara"));
		TArray<FTaggedNiagaraRow*> Rows;
		NiagaraDataTable->GetAllRows<FTaggedNiagaraRow>(Context, Rows);

		for (const FTaggedNiagaraRow* Row : Rows)
		{
			if (Row && Row->NiagaraTag.IsValid() && !Row->NiagaraAsset.IsNull())
			{
				NiagaraInfos.Add(Row->NiagaraTag, Row->NiagaraAsset);
			}
		}
	}
}

void UFXManagerSubsystem::Deinitialize()
{
	FScopeLock Lock(&PendingRequestsLock);
	// 안전하게 종료하기 위해 현재 로드 중인 에셋을 모두 취소합니다.
	PendingSoundLoadRequests.Empty();
	Super::Deinitialize();
}

void UFXManagerSubsystem::AsyncPlaySoundAtLocation(const FGameplayTag SoundTag, const FVector Location, const FRotator Rotation, float VolumeMultiplier, float PitchMultiplier)
{
	FScopeLock Lock(&PendingRequestsLock);
	
	const TSoftObjectPtr<USoundBase> SoundToLoad = SoundInfos.FindRef(SoundTag);
	if (SoundToLoad.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("SoundTag %s에 해당하는 사운드를 찾을 수 없거나 유효하지 않습니다."), *SoundTag.ToString());
		return;
	}
	
	// 이미 에셋이 로드되어있는 경우 들어가는 분기입니다.
	if (USoundBase* LoadedSound = SoundToLoad.Get())
	{
		if (GetWorld())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), LoadedSound, Location, Rotation, VolumeMultiplier, PitchMultiplier);
		}
		return;
	}
	
	FSoftObjectPath AssetPath = SoundToLoad.ToSoftObjectPath();
	
	// 이미 로딩 중인 경우 들어가는 분기입니다.
	if (FSoundAsyncLoadRequest* ExistingRequest = PendingSoundLoadRequests.Find(AssetPath))
	{
		// 로딩 중인 에셋이 끝날 때 이 요청에 대해서도 함께 재생하기 위해 배열에 추가합니다.
		ExistingRequest->LocationsToPlay.Add(Location);
		ExistingRequest->RotationsToPlay.Add(Rotation);
		ExistingRequest->VolumeMultiplier.Add(VolumeMultiplier);
		ExistingRequest->PitchMultiplier.Add(PitchMultiplier);
		
		return;
	}

	// 새로 로드를 시작해야 하는 경우 들어가는 분기입니다.
	if (!StreamableManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("StreamableManager가 유효하지 않습니다."))
		return;
	}

	// 함수를 바인드하기 위한 변수를 선언 및 초기화합니다.
	FSoundAsyncLoadRequest NewRequest;
	NewRequest.LocationsToPlay.Add(Location);
	NewRequest.RotationsToPlay.Add(Rotation);
	NewRequest.VolumeMultiplier.Add(VolumeMultiplier);
	NewRequest.PitchMultiplier.Add(PitchMultiplier);

	// 에셋 로드가 끝난 뒤 호출되는 델리게이트에 함수를 바인드합니다.
	FStreamableDelegate StreamableCompleteDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnSoundAsyncLoadComplete, AssetPath);
	NewRequest.StreamableHandle.Emplace(StreamableManager->RequestAsyncLoad(AssetPath, StreamableCompleteDelegate));

	PendingSoundLoadRequests.Add(AssetPath, NewRequest);
}

void UFXManagerSubsystem::OnSoundAsyncLoadComplete(FSoftObjectPath LoadedAssetPath)
{
	FScopeLock Lock(&PendingRequestsLock);
	
	// 로드 완료 후 사운드 재생을 시작합니다.
	USoundBase* LoadedSound = Cast<USoundBase>(LoadedAssetPath.ResolveObject());

	if (FSoundAsyncLoadRequest* CompletedRequest = PendingSoundLoadRequests.Find(LoadedAssetPath))
	{
		if (LoadedSound && GetWorld())
		{
			for (int32 i = 0; i < CompletedRequest->LocationsToPlay.Num(); ++i)
			{
				const FVector PlayLocation = CompletedRequest->LocationsToPlay[i];
				const FRotator PlayRotation = CompletedRequest->RotationsToPlay[i];
				const float PlayVolumeMultiplier = CompletedRequest->VolumeMultiplier[i];
				const float PlayPitchMultiplier = CompletedRequest->PitchMultiplier[i];
               
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), LoadedSound, PlayLocation, PlayRotation, PlayVolumeMultiplier, PlayPitchMultiplier);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("로딩 후, USoundBase가 유효하지 않거나 월드를 찾을 수 없습니다."));
		}

		PendingSoundLoadRequests.Remove(LoadedAssetPath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("로딩 후, 등록된 콜백 요청을 찾을 수 없습니다."));
	}
}

void UFXManagerSubsystem::AsyncPlayNiagaraAtLocation(const FGameplayTag NiagaraTag, const FVector Location, const FRotator Rotation, const FVector Scale, bool bAutoDestroy, bool bAutoActivate)
{
	FScopeLock Lock(&PendingRequestsLock);
	
	const TSoftObjectPtr<UNiagaraSystem> NiagaraToLoad = NiagaraInfos.FindRef(NiagaraTag);
	if (NiagaraToLoad.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("NiagaraTag %s에 해당하는 나이아가라를 찾을 수 없거나 유효하지 않습니다."), *NiagaraTag.ToString());
		return;
	}
	
	if (UNiagaraSystem* LoadedNiagara = NiagaraToLoad.Get())
	{
		if (GetWorld())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LoadedNiagara, Location, Rotation, Scale, bAutoDestroy, bAutoActivate);
		}
		return;
	}
	
	FSoftObjectPath AssetPath = NiagaraToLoad.ToSoftObjectPath();
	
	if (FNiagaraAsyncLoadRequest* ExistingRequest = PendingNiagaraLoadRequests.Find(AssetPath))
	{
		ExistingRequest->LocationsToPlay.Add(Location);
		ExistingRequest->RotationsToPlay.Add(Rotation);
		ExistingRequest->ScalesToPlay.Add(Scale);
		ExistingRequest->bAutoDestroy.Add(bAutoDestroy);
		ExistingRequest->bAutoActivate.Add(bAutoActivate);

		return;
	}

	if (!StreamableManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("StreamableManager가 유효하지 않습니다."))
		return;
	}

	FNiagaraAsyncLoadRequest NewRequest;
	NewRequest.LocationsToPlay.Add(Location);
	NewRequest.RotationsToPlay.Add(Rotation);
	NewRequest.ScalesToPlay.Add(Scale);
	NewRequest.bAutoDestroy.Add(bAutoDestroy);
	NewRequest.bAutoActivate.Add(bAutoActivate);

	FStreamableDelegate StreamableCompleteDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnNiagaraAsyncLoadComplete, AssetPath);
	NewRequest.StreamableHandle.Emplace(StreamableManager->RequestAsyncLoad(AssetPath, StreamableCompleteDelegate));

	PendingNiagaraLoadRequests.Add(AssetPath, NewRequest);
}

void UFXManagerSubsystem::OnNiagaraAsyncLoadComplete(FSoftObjectPath LoadedAssetPath)
{
	FScopeLock Lock(&PendingRequestsLock);

	UNiagaraSystem* LoadedNiagara = Cast<UNiagaraSystem>(LoadedAssetPath.ResolveObject());

	if (FNiagaraAsyncLoadRequest* CompletedRequest = PendingNiagaraLoadRequests.Find(LoadedAssetPath))
	{
		if (LoadedNiagara && GetWorld())
		{
			for (int32 i = 0; i < CompletedRequest->LocationsToPlay.Num(); ++i)
			{
				const FVector PlayLocation = CompletedRequest->LocationsToPlay[i];
				const FRotator PlayRotation = CompletedRequest->RotationsToPlay[i];
				const FVector PlayScale = CompletedRequest->ScalesToPlay[i];
				const bool bAutoDestroy = CompletedRequest->bAutoDestroy[i];
				const bool bAutoActivate = CompletedRequest->bAutoActivate[i];

				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LoadedNiagara, PlayLocation, PlayRotation, PlayScale, bAutoDestroy, bAutoActivate);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("로딩 후, UNiagaraSystem이 유효하지 않거나 월드를 찾을 수 없습니다."));
		}

		PendingNiagaraLoadRequests.Remove(LoadedAssetPath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("로딩 후, 등록된 콜백 요청을 찾을 수 없습니다."));
	}
}

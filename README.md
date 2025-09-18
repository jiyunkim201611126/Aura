# Aura

언리얼 리슨 서버 기반의 GAS 학습용 프로젝트입니다.

- 엔진: 5.6
- IDE: Rider
- 인원: 1인개발

# 주요 기술

## Composite Pattern

관련 블로그 게시글: [#1](https://velog.io/@jiyunkim/%EB%94%B8%EA%B9%8D%EC%9C%BC%EB%A1%9C-%EC%B6%A9%EC%A0%84%EC%8B%9D-Ability-%EA%B5%AC%ED%98%84) [#2](https://velog.io/@jiyunkim/%EB%94%B8%EA%B9%8D%EC%9C%BC%EB%A1%9C-%EC%8A%A4%ED%83%9D%ED%98%95-Ability-%EA%B5%AC%ED%98%84-2) [#3](https://velog.io/@jiyunkim/%EB%94%B8%EA%B9%8D%EB%94%B8%EA%B9%8D%EB%94%B8%EA%B9%8D%EB%94%B8%EA%B9%8D%EB%94%B8%EA%B9%8D%EC%9C%BC%EB%A1%9C-%EC%8A%A4%ED%83%9D%ED%98%95-Ability-%EA%B5%AC%ED%98%84-3) [#4](https://velog.io/@jiyunkim/%EB%94%B8%EA%B9%8D%EC%9C%BC%EB%A1%9C-%EC%B6%A9%EC%A0%84%ED%98%95-Ability-%EA%B5%AC%ED%98%84-4-%ED%95%A9%EC%84%B1-%ED%8C%A8%ED%84%B4-%EC%A0%84%EB%9E%B5-%ED%8C%A8%ED%84%B4)

<img width="459" height="217" alt="image" src="https://github.com/user-attachments/assets/95a406c8-25e7-420f-b355-322aa371bfc3" />

위처럼 Ability 블루프린트 클래스에 값을 할당하면 즉시 '사용 횟수 충전형(Stackable) Ability'로 변경되는 기능입니다.

```cpp
// AuraAbilitySystemComponent.h
UPROPERTY(Replicated)
TArray<TObjectPtr<AActor>> AdditionalCostManagers;
```

Stackable Ability를 하나라도 소지하고 있다면 Manager 객체가 자동 생성되며, ASC에 선언되어 COND_OwnerOnly로 복제되는 멤버 변수에 할당됩니다.

```cpp
// StackableAbilityManager.h
USTRUCT()
struct FAbilityStackArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FAbilityStackItem> Items;

	UPROPERTY(NotReplicated)
	class AStackableAbilityManager* OwnerActor = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FAbilityStackItem, FAbilityStackArray>(Items, DeltaParams, *this);
	}
};
template<>
struct TStructOpsTypeTraits<FAbilityStackArray> : public TStructOpsTypeTraitsBase2<FAbilityStackArray>
{
	enum { WithNetDeltaSerializer = true };
};
```

해당 매니저 클래스는 FastArraySerializer로 Stackable Ability들의 충전 로직을 관리하며, 클라이언트에게 복제해줍니다.

<img width="582" height="366" alt="image" src="https://github.com/user-attachments/assets/c632eff9-1e66-4923-a791-282a978f8c3a" />

GameplayEffect(데미지나 디버프 등) 적용 여부 또한 합성 패턴으로 쉽게 추가/제거할 수 있습니다.

## MVVM Pattern

```cpp
// StackableAbilityManager.cpp
void FAbilityStackItem::PostReplicatedChange(const FAbilityStackArray& InArraySerializer)
{
	if (!InArraySerializer.OwnerActor)
	{
		return;
	}
	
	InArraySerializer.OwnerActor->OnStackCountChanged.ExecuteIfBound(AbilityTag, CurrentStack);

	if (bShouldTimerStart)
	{
		InArraySerializer.OwnerActor->OnStackTimerStarted.ExecuteIfBound(AbilityTag, RechargeTime, RechargeTime);
	}
}
```
<img width="49" height="49" alt="image" src="https://github.com/user-attachments/assets/bbdd2cb5-dc85-47f8-bdfa-380fe0bdd7cf" />

HUD는 MVVM 패턴으로 구성되어 있으며, 리슨 서버와 클라이언트 모두 콜백 함수에 의해 해당 AdditionalCost 정보가 HUD에 반영됩니다.
스택 충전 상황 외에도 스탯창, 스킬창 등 ASC와 Attribute 관련 UI에도 MVVM 패턴이 적용되어 있습니다.

## Behavior Tree, Sub Tree

<img width="724" height="696" alt="image" src="https://github.com/user-attachments/assets/c9d48e99-a98e-4901-a60d-be6e3644b806" />

기본 공격만 하는 일반 AI와 달리, 하수인 소환이 불가능한 상황에만 기본 공격을 실행하는 보스 AI가 Sub Tree를 활용해 구성되어 있습니다.

## 태그 기반 에셋 중앙 관리, 비동기 로드를 통한 런타임 성능 최적화

```cpp
// FXManagerSubsystem.h
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

	// DataTable에 매핑되어있는 Tag와 에셋들은 탐색 효율을 위해 TMap으로 재구성되므로, 메모리 효율을 위해 Soft로 선언합니다.
	UPROPERTY(Config)
	TSoftObjectPtr<UDataTable> SoundDataTablePath;
	
	UPROPERTY(Config)
	TSoftObjectPtr<UDataTable> NiagaraDataTablePath;

	UPROPERTY()
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> SoundMap;
	
	UPROPERTY()
	TMap<FGameplayTag, TSoftObjectPtr<UNiagaraSystem>> NiagaraMap;
```

메모리와 탐색 성능을 모두 잡은 시스템 구성입니다.
Initialize에서 DataTable을 한 번에 로드해 TMap으로 GameplayTag와 에셋을 매핑하여 캐싱합니다.
비동기 로드를 통한 FX 재생 요청 시, 요청하는 클래스에선 Tag와 함께 재생에 필요한 정보를 보냅니다.
Get 함수의 경우 요청 시 콜백 함수를 보내서 로드 완료 시점에 매개변수로 담아 보냅니다.

```cpp
// FXManagerSubsystem.cpp
void UFXManagerSubsystem::AsyncPlaySoundAtLocation(const FGameplayTag& SoundTag, const FVector Location, const FRotator Rotation, const float VolumeMultiplier, const float PitchMultiplier)
{
	if (!SoundTag.IsValid() || !StreamableManager)
	{
		return;
	}
	
	FScopeLock Lock(&PendingRequestsLock);
	
	const TSoftObjectPtr<USoundBase> SoundToLoad = SoundMap.FindRef(SoundTag);
	if (SoundToLoad.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("SoundTag %s에 해당하는 사운드를 찾을 수 없습니다."), *SoundTag.ToString());
		return;
	}
	
	// 이미 에셋이 로드되어있는 경우 들어가는 분기입니다.
	if (SoundToLoad.IsValid())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToLoad.Get(), Location, Rotation, VolumeMultiplier, PitchMultiplier);
		return;
	}
	
	FSoftObjectPath AssetPath = SoundToLoad.ToSoftObjectPath();
	
	// 함수를 바인드하기 위한 변수를 선언 및 초기화합니다.
	FSoundAsyncPlayData NewPlayData;
	NewPlayData.LocationsToPlay = Location;
	NewPlayData.RotationsToPlay = Rotation;
	NewPlayData.VolumeMultiplier = VolumeMultiplier;
	NewPlayData.PitchMultiplier = PitchMultiplier;
	
	// 이미 로드 중인 경우 들어가는 분기입니다.
	if (FSoundAsyncLoadRequest* ExistingRequest = PendingSoundLoadRequests.Find(AssetPath))
	{
		// 로드 중인 에셋이 로딩 완료 시점에 이 요청에 대해서도 함께 처리하기 위해 배열에 추가합니다.
		ExistingRequest->PlayRequests.Add(NewPlayData);
		return;
	}

	// 새로 로드를 시작해야 하는 경우 여기로 내려옵니다.
	// 에셋 로드가 끝난 뒤 호출되는 델리게이트에 함수를 바인드합니다.
	FSoundAsyncLoadRequest NewRequest;
	NewRequest.PlayRequests.Add(NewPlayData);	
	FStreamableDelegate StreamableCompleteDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnSoundAsyncLoadComplete, AssetPath);
	StreamableManager->RequestAsyncLoad(AssetPath, StreamableCompleteDelegate);

	PendingSoundLoadRequests.Add(AssetPath, NewRequest);
}
```

위 함수는 사운드 재생 요청에 대응하는 함수입니다.
이미 로드되어 있는 에셋인 경우, 로드 중인 에셋인 경우, 새로 로드를 시작해야 하는 경우 모두 대응하고 있습니다.
같은 에셋에 대한 여러 요청이 동시에 들어오는 경우에도 문제 없이 재생됩니다.

```cpp
// FXManagerSubsystem.cpp
void UFXManagerSubsystem::AsyncGetSound(const FGameplayTag& SoundTag, const TFunction<void(USoundBase*)>& OnLoadedCallback)
{
	// 위 함수와 마찬가지로 비동기 로드를 요청하는 함수입니다.
	// 차이점은 SoundBase을 반환받는다는 데에 있습니다.
	// AuraProjectile의 BeginPlay에 예시가 있습니다.
	// 나이아가라 예시는 AuraNiagaraComponent의 OnRep_DebuffTag에 있습니다.
	if (!SoundTag.IsValid() || !StreamableManager)
	{
		return;
	}
	
	FScopeLock Lock(&PendingRequestsLock);
	
	const TSoftObjectPtr<USoundBase> SoundToLoad = SoundMap.FindRef(SoundTag);
	if (SoundToLoad.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("SoundTag %s에 해당하는 사운드를 찾을 수 없습니다."), *SoundTag.ToString());
		return;
	}

	const FSoftObjectPath AssetPath = SoundToLoad.ToSoftObjectPath();

	// 이미 에셋이 로드되어있는 경우 들어가는 분기입니다.
	if (SoundToLoad.IsValid())
	{
		OnLoadedCallback(SoundToLoad.Get());
		return;
	}

	// 이미 로드 중인 경우 콜백 함수만 등록하고 리턴합니다.
	if (PendingSoundLoadRequests.Contains(AssetPath))
	{
		PendingSoundLoadRequests[AssetPath].GetterCallbacks.Add(OnLoadedCallback);
		return;
	}

	// 처음 요청된 태그인 경우 콜백 리스트를 생성합니다.
	FSoundAsyncLoadRequest NewRequest;
	NewRequest.GetterCallbacks.Add(OnLoadedCallback);
	FStreamableDelegate StreamableCompleteDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnSoundAsyncLoadComplete, AssetPath);
	StreamableManager->RequestAsyncLoad(AssetPath, StreamableCompleteDelegate);
	
	PendingSoundLoadRequests.Add(AssetPath, NewRequest);
}
```

Getter 함수의 경우도 마찬가지로 세 가지 상황에 모두 대응되며, 재생 요청 함수와 로드 완료 타이밍을 공유합니다.

```cpp
// FXManagerSubsystem.h
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
```

로드 완료 시 두 개의 배열을 모두 순회하는 것으로, 모든 요청에 한 번에 대응합니다.

```cpp
// GC_NiagaraWithLocationArray.cpp
bool UGC_NiagaraWithLocationArray::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	for (const FVector_NetQuantize& Location : UAuraAbilitySystemLibrary::GetLocationsFromContext(Parameters.EffectContext))
	{
		if (UFXManagerSubsystem* FXManager = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>())
		{
			FXManager->AsyncSpawnNiagaraAtLocation(Parameters.AggregatedSourceTags.GetByIndex(0), Location);
		}
	}
	
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
```

FXManagerSubsystem은 GameplayCue에서도 활용 가능합니다.

#include "AuraEnemySpawnPoint.h"

#include "Aura/Character/AuraEnemy.h"

AAuraEnemySpawnPoint::AAuraEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAuraEnemySpawnPoint::SpawnEnemy()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AAuraEnemy* Enemy = GetWorld()->SpawnActorDeferred<AAuraEnemy>(EnemyClass, GetActorTransform());
	Enemy->SetLevel(EnemyLevel);
	// 스폰된 적 캐릭터 사망 시 리스폰할 수 있도록 콜백을 통해 타이머를 시작합니다.
	Enemy->GetOnDeathDelegate().AddDynamic(this, &ThisClass::StartSpawnTimer);
	Enemy->FinishSpawning(GetActorTransform());
	Enemy->SpawnDefaultController();
	
	EndSpawnTimer();
}

void AAuraEnemySpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		SpawnEnemy();
	}
}

void AAuraEnemySpawnPoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndSpawnTimer();
	Super::EndPlay(EndPlayReason);
}

void AAuraEnemySpawnPoint::StartSpawnTimer(AActor* DeadActor)
{
	if (HasAuthority())
	{
		FTimerDelegate TimerDelegate;
		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, [this]()
		{
			SpawnEnemy();
		},
		SpawnPeriod, false);
	}
}

void AAuraEnemySpawnPoint::EndSpawnTimer()
{
	if (HasAuthority() && SpawnTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(SpawnTimerHandle);
		}
	}
}

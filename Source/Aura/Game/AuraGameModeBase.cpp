#include "AuraGameModeBase.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "AuraGameInstance.h"
#include "Aura/Manager/SaveManagerSubsystem.h"

TSoftObjectPtr<UWorld> AAuraGameModeBase::GetMap(const FString& MapName)
{
	return Maps.FindChecked(MapName);
}

void AAuraGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	const USaveManagerSubsystem* SaveManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	Maps.Add(SaveManagerSubsystem->DefaultMapName, DefaultMap);
}

AActor* AAuraGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);

	if (Actors.Num() > 0)
	{
		AActor* SelectedActor = Actors[0];
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				// PlayerStart의 Tag 중 GameInstance에 캐싱된 Tag와 일치하는 PlayerStart를 탐색합니다. 
				if (PlayerStart->PlayerStartTag == AuraGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}
	return nullptr;
}

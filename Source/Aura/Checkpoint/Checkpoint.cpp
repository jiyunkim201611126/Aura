#include "Checkpoint.h"

#include "Aura/Interaction/SaveGameInterface.h"
#include "Aura/Manager/SaveManagerSubsystem.h"
#include "Components/SphereComponent.h"

ACheckpoint::ACheckpoint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>("CheckpointMesh");
	CheckpointMesh->SetupAttachment(GetRootComponent());
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CheckpointMesh->SetCollisionResponseToAllChannels(ECR_Block);

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(CheckpointMesh);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ACheckpoint::LoadActor_Implementation()
{
	if (bReached)
	{
		ActiveGlowEffects();
	}
}

void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
}

void ACheckpoint::Destroyed()
{
	Sphere->OnComponentBeginOverlap.Clear();
	
	Super::Destroyed();
}

void ACheckpoint::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APawn* OtherPawn = Cast<APawn>(OtherActor))
	{
		if (OtherPawn->IsLocallyControlled() && OtherPawn->Implements<USaveGameInterface>())
		{
			// 저장 로직을 수행합니다.
			ISaveGameInterface::Execute_SaveProgress(OtherPawn, PlayerStartTag);
			
			bReached = true;
			if (USaveManagerSubsystem* SaveManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
			{
				SaveManagerSubsystem->SaveWorldState(GetWorld());
			}
			
			ActiveGlowEffects();
		}
	}
}

void ACheckpoint::ActiveGlowEffects()
{
	UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(CheckpointMesh->GetMaterial(0), this);
	CheckpointMesh->SetMaterial(0, DynamicMaterialInstance);
	CheckpointReached(DynamicMaterialInstance);
}

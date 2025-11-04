#include "Checkpoint.h"

#include "Aura/Interaction/SaveGameInterface.h"
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
			// Checkpoint의 StartTag를 저장합니다. 
			ISaveGameInterface::Execute_SaveProgress(OtherPawn, PlayerStartTag);
			
			ActiveGlowEffects();
	
			// 이미 한 번 작동한 Checkpoint이므로, SphereComponent가 더이상 이벤트를 발생시킬 수 없도록 설정합니다.
			Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Sphere->OnComponentBeginOverlap.Clear();
		}
	}
}

void ACheckpoint::ActiveGlowEffects()
{
	UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(CheckpointMesh->GetMaterial(0), this);
	CheckpointMesh->SetMaterial(0, DynamicMaterialInstance);
	CheckpointReached(DynamicMaterialInstance);
}

#include "MapEntrance.h"

#include "Aura/Aura.h"
#include "Aura/Player/AuraPlayerController.h"
#include "Components/SphereComponent.h"

AMapEntrance::AMapEntrance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	MapEntranceMesh = CreateDefaultSubobject<UStaticMeshComponent>("CheckpointMesh");
	MapEntranceMesh->SetupAttachment(GetRootComponent());
	MapEntranceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MapEntranceMesh->SetCollisionResponseToAllChannels(ECR_Block);
	
	MapEntranceMesh->CustomDepthStencilValue = CUSTOM_DEPTH_BLUE;
	MapEntranceMesh->MarkRenderStateDirty();

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(MapEntranceMesh);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AMapEntrance::HighlightActor()
{
	MapEntranceMesh->SetRenderCustomDepth(true);
}

void AMapEntrance::UnHighlightActor()
{
	MapEntranceMesh->SetRenderCustomDepth(false);
}

void AMapEntrance::BeginPlay()
{
	Super::BeginPlay();

	if (Sphere)
	{
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	}
}

void AMapEntrance::Destroyed()
{
	if (Sphere)
	{
		Sphere->OnComponentBeginOverlap.Clear();
	}
	
	Super::Destroyed();
}

void AMapEntrance::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (const APawn* OtherPawn = Cast<APawn>(OtherActor))
		{
			if (OtherPawn->IsLocallyControlled())
			{
				if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(OtherPawn->GetController()))
				{
					AuraPlayerController->ServerRequestTravelWithMapAsset(DestinationMap, DestinationPlayerStartTag);
				}
			}
		}
	}
}

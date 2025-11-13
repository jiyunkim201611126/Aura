#pragma once

#include "CoreMinimal.h"
#include "Aura/Interaction/HighlightInterface.h"
#include "GameFramework/Actor.h"
#include "MapEntrance.generated.h"

class USphereComponent;

UCLASS()
class AURA_API AMapEntrance : public AActor, public IHighlightInterface
{
	GENERATED_BODY()

public:
	AMapEntrance(const FObjectInitializer& ObjectInitializer);
	
	//~ Begin IHighlightInterface
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	//~ End of IHighlightInterface

protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	//~ End of AActor Interface
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
protected:
	UPROPERTY(EditAnywhere, Category = "MapEntrance")
	TSoftObjectPtr<UWorld> DestinationMap;

	UPROPERTY(EditAnywhere, Category = "MapEntrance")
	FName DestinationPlayerStartTag;
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MapEntranceMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};

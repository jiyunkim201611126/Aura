#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Checkpoint.generated.h"

class USphereComponent;

UCLASS()
class AURA_API ACheckpoint : public APlayerStart
{
	GENERATED_BODY()

public:
	ACheckpoint(const FObjectInitializer& ObjectInitializer);

protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End of AActor Interface
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent)
	void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

	void ActiveGlowEffects();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};

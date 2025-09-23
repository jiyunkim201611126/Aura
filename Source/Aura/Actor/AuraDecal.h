#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraDecal.generated.h"

UCLASS()
class AURA_API AAuraDecal : public AActor
{
	GENERATED_BODY()

public:
	AAuraDecal();
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDecalComponent> DecalComponent;
};

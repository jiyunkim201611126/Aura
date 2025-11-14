#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageTextActor.generated.h"

class UProjectileMovementComponent;
enum class EDamageTypeContext : uint8;

UCLASS()
class AURA_API ADamageTextActor : public AActor
{
	GENERATED_BODY()
	
public:
	ADamageTextActor();
	
	UFUNCTION(BlueprintImplementableEvent)
	void InitDamageText(float Damage, bool bBlockedHit, bool bCriticalHit, const EDamageTypeContext DamageType);

protected:
	//~ Begin Actor Interface
	virtual void BeginPlay() override;
	//~ End Actor Interface
	
private:
	void InitMovement();

private:
	UPROPERTY(EditDefaultsOnly)
	float InitialSpeed = 100.0f;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
};

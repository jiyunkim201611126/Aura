#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageTextActor.generated.h"

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
	virtual void Tick(float DeltaSeconds) override;
	//~ End Actor Interface
	
private:
	void InitMovement();

private:
	UPROPERTY(EditDefaultsOnly)
	float InitialSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly)
	float Gravity = 400.0f;

	UPROPERTY(EditDefaultsOnly)
	float DescentGravityScale = 0.5f;

	// 현재 속도입니다.
	FVector Velocity;
	
	// 하강 상태 여부입니다.
	bool bIsFalling = false;
};

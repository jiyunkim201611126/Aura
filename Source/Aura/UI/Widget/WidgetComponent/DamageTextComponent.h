#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DamageTextComponent.generated.h"

enum class EDamageTypeContext : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURA_API UDamageTextComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintImplementableEvent)
	void SetDamageText(float Damage, bool bBlockedHit, bool bCriticalHit, const EDamageTypeContext DamageType);
	
	// ~UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// ~End of UActorComponent Interface

private:
	void InitMovement();

private:
	UPROPERTY(EditDefaultsOnly)
	float InitialSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly)
	float Gravity = 400.0f;

	UPROPERTY(EditDefaultsOnly)
	float DescentGravityScale = 0.5f;

	// 현재 속도
	FVector Velocity;
	
	// 하강 상태 여부
	bool bIsFalling = false;
};
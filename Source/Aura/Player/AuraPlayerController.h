#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;
class UAuraInputConfig;
struct FGameplayTag;
class UAuraAbilitySystemComponent;
class USplineComponent;
class UDamageTextComponent;

UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();

	// Damage를 보여주는 위젯 컴포넌트를 스폰하는 함수
	void SpawnDamageText(float DamageAmount, AActor* TargetActor, bool bBlockedHit, bool bCriticalHit) const;

protected:
	// ~AActor Interface
	virtual void BeginPlay() override;
	// ~End of AActor Interface
	
	// ~AController Interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	// ~End of AController Interface

	// ~APlayerController Interface
	virtual void SetupInputComponent() override;
	// ~End of APlayerController Interface

private:
	void CursorTrace();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void ShiftPressed() { bShiftKeyDown = true; }
	void ShiftReleased() { bShiftKeyDown = false; }

	void Move(const FInputActionValue& InputActionValue);
	void AutoRun();

	UAuraAbilitySystemComponent* GetASC();

public:
	FHitResult CursorHit;
	
private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;
	
	bool bShiftKeyDown = false;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	// 마우스 입력을 통한 Move를 구현하기 위한 관련 변수
	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bTargeting = false;
	bool bAutoRunning = false;
	float AutoRunAcceptanceRadius = 50.f;

	// FVector들을 추가해주면 자동으로 곡선 경로를 생성해주는 컴포넌트
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
};

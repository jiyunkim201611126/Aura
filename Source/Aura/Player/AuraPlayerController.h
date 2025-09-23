#pragma once

#include "CoreMinimal.h"
#include "Aura/Interaction/SkillPreviewInterface.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"

class AAuraDecal;
class ADamageTextActor;
class UNiagaraSystem;
enum class EDamageTypeContext : uint8;
class UInputMappingContext;
class UInputAction;
class IEnemyInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;
struct FInputActionValue;
struct FGameplayTag;

UCLASS()
class AURA_API AAuraPlayerController : public APlayerController, public ISkillPreviewInterface
{
	GENERATED_BODY()

public:
	AAuraPlayerController();

	// Damage를 보여주는 위젯 컴포넌트를 스폰하는 함수입니다.
	UFUNCTION(Client, Reliable)
	void ClientSpawnDamageText(float DamageAmount, AActor* TargetActor, bool bBlockedHit, bool bCriticalHit, const EDamageTypeContext DamageType);

	//~ Begin ISkillPreview Interface
	virtual void ShowSkillPreview_Implementation(UMaterialInterface* DecalMaterial = nullptr) override;
	virtual void HideSkillPreview_Implementation() override;
	//~ End ISkillPreview Interface

protected:
	//~ Begin Actor Interface
	virtual void BeginPlay() override;
	//~ End Actor Interface
	
	//~ Begin Controller Interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~ End Controller Interface

	//~ Begin PlayerController Interface
	virtual void SetupInputComponent() override;
	//~ End APlayerController Interface

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

	// 마우스 입력을 통한 Move를 구현하기 위한 관련 변수들입니다.
	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bTargeting = false;
	bool bAutoRunning = false;
	float AutoRunAcceptanceRadius = 50.f;

	// FVector들을 추가해주면 자동으로 곡선 경로를 생성해주는 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ADamageTextActor> DamageTextClass;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAuraDecal> SkillPreviewClass;

	UPROPERTY()
	TObjectPtr<AAuraDecal> SkillPreview;

	void UpdateSkillPreviewLocation();
};

#pragma once

#include "CoreMinimal.h"
#include "Aura/Interaction/SkillPreviewInterface.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"

class AAuraDecal;
class ADamageTextActor;
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class IHighlightInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;
struct FInputActionValue;
struct FGameplayTag;
enum class EDamageTypeContext : uint8;

UENUM()
enum class ETargetingStatus : uint8
{
	TargetingEnemy,
	TargetingNonEnemy,
	NotTargeting,
};

UCLASS()
class AURA_API AAuraPlayerController : public APlayerController, public ISkillPreviewInterface
{
	GENERATED_BODY()

public:
	AAuraPlayerController();

	UFUNCTION(Server, Reliable)
	void ServerRequestTravel(const FString& MapName);

	UFUNCTION(Client, Reliable)
	void ClientResponseTravel(const TSoftObjectPtr<UWorld>& MapToTravel);

	// Damage를 보여주는 위젯 컴포넌트를 스폰하는 함수입니다.
	UFUNCTION(Client, Reliable)
	void ClientSpawnDamageText(float DamageAmount, AActor* TargetActor, bool bBlockedHit, bool bCriticalHit, const EDamageTypeContext DamageType);

	//~ Begin ISkillPreview Interface
	virtual void ShowSkillPreview_Implementation(UMaterialInterface* DecalMaterial = nullptr) override;
	virtual void HideSkillPreview_Implementation() override;
	//~ End of ISkillPreview Interface

protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End of AActor Interface
	
	//~ Begin AController Interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~ End of AController Interface

	//~ Begin APlayerController Interface
	virtual void SetupInputComponent() override;
	//~ End of APlayerController Interface

private:
	// 커서 아래의 대상을 하이라이팅하며, 그 대상을 멤버변수로 캐싱하는 함수입니다.
	void CursorTrace();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void ShiftPressed() { bShiftKeyDown = true; }
	void ShiftReleased() { bShiftKeyDown = false; }

	void Move(const FInputActionValue& InputActionValue);
	void AutoRun();

	UAuraAbilitySystemComponent* GetASC();

	void UpdateSkillPreviewLocation() const;

public:
	FHitResult CursorHit;
	
private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	TScriptInterface<IHighlightInterface> LastActor;
	TScriptInterface<IHighlightInterface> ThisActor;

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
	ETargetingStatus TargetingStatus = ETargetingStatus::NotTargeting;
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
};

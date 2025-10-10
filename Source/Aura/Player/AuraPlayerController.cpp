#include "AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "Aura/Input/AuraInputComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "NiagaraFunctionLibrary.h"
#include "Aura/Aura.h"
#include "Aura/Actor/AuraDecal.h"
#include "Aura/Actor/DamageTextActor.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Components/DecalComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::ShowSkillPreview_Implementation(UMaterialInterface* DecalMaterial)
{
	if (!IsLocalController())
	{
		return;
	}
	
	Execute_HideSkillPreview(this);
	SkillPreview = GetWorld()->SpawnActor<AAuraDecal>(SkillPreviewClass);
	if (SkillPreview && DecalMaterial)
	{
		SkillPreview->DecalComponent->SetMaterial(0, DecalMaterial);
	}
}

void AAuraPlayerController::HideSkillPreview_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}
	
	if (SkillPreview)
	{
		SkillPreview->Destroy();
	}
}

void AAuraPlayerController::ClientSpawnDamageText_Implementation(float DamageAmount, AActor* TargetActor, bool bBlockedHit, bool bCriticalHit, const EDamageTypeContext DamageType)
{
	if (TargetActor && DamageTextClass && IsLocalController())
	{
		if (ADamageTextActor* DamageText = GetWorld()->SpawnActor<ADamageTextActor>(DamageTextClass))
		{
			DamageText->SetActorTransform(TargetActor->GetActorTransform());
			DamageText->InitDamageText(DamageAmount, bBlockedHit, bCriticalHit, DamageType);
		}
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
	AutoRun();
	UpdateSkillPreviewLocation();
}

void AAuraPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HasAuthority())
	{
		Cast<ICombatInterface>(InPawn)->RegisterPawn();
	}
}

void AAuraPlayerController::OnUnPossess()
{
	if (HasAuthority())
	{
		Cast<ICombatInterface>(GetPawn())->UnregisterPawn();
	}
	
	Super::OnUnPossess();
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// InputComponent는 ProjectSettings의 Input 탭에서 지정합니다.
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);

	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);

	// InputComponent에게 InputConfig(DataAsset)과 함수 포인터들을 전달합니다.
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagHeld, &ThisClass::AbilityInputTagReleased);
}

void AAuraPlayerController::CursorTrace()
{
	// 커서 아래의 적을 하이라이팅하며 그 위치를 멤버변수로 캐싱하는 함수입니다.

	// CursorTrace가 Block된 상태라면 모두 UnHighlight합니다.
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_CursorTrace))
	{
		if (LastActor)
		{
			LastActor->UnHighlightActor();
		}
		if (ThisActor)
		{
			ThisActor->UnHighlightActor();
		}
		
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	
	GetHitResultUnderCursor(ECC_Target, false, CursorHit);
	
	if (!CursorHit.bBlockingHit)
	{
		if (LastActor)
		{
			LastActor->UnHighlightActor();
		}
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();
	
	if (LastActor != ThisActor)
	{
		if (LastActor)
		{
			LastActor->UnHighlightActor();
		}
		if (ThisActor)
		{
			ThisActor->HighlightActor();
		}
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	
	// RMB Input인 경우 들어가는 분기입니다.
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		// 현재 마우스 아래에 Enemy가 존재하면 bTargeting를 true로 할당합니다.
		bTargeting = ThisActor ? true : false;
		// 꾹 누른 시간을 0으로 초기화합니다.
		FollowTime = 0.f;
		return;
	}

	// 이동이 아닌 다른 입력이 들어온 경우 이동을 중단합니다.
	bAutoRunning = false;

	if (GetASC())
	{
		GetASC()->AbilityInputTagPressed(InputTag);
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	
	// RMB Input이 아닌 경우 들어가는 분기입니다.
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	// RMB Input인 경우 여기에 도달합니다.
	if (bTargeting || bShiftKeyDown)
	{
		// 마우스 아래 Enemy가 있거나 Shift를 누른 상태면 Ability를 사용합니다.
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);

			// 이동을 중단합니다.
			bAutoRunning = false;
		}
	}
	else
	{
		// 마우스 아래 Enemy가 없으면 Move를 원한다고 간주, 얼마나 오랜 시간 동안 누르고 있었는지 기록을 시작합니다.
		FollowTime += GetWorld()->GetDeltaSeconds();

		if (CursorHit.bBlockingHit)
		{
			CachedDestination = CursorHit.ImpactPoint;
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}
	
	// ASC가 입력이 Release되었는지 알아야 하기 때문에 일단 호출합니다.
	if (GetASC())
	{
		GetASC()->AbilityInputTagReleased(InputTag);
	}
	
	// RMB Input이 아닌 경우 즉시 return합니다.
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		return;
	}
	
	// RMB Input인 경우 여기에 도달합니다.

	// 커서 아래에 적이 없으며 Shift를 누른 상태가 아니면 들어가는 분기입니다.
	if (!bTargeting && !bShiftKeyDown)
	{
		// 마우스 아래 Enemy가 없으면 Move를 원한다고 간주합니다.
		const APawn* ControlledPawn = GetPawn();
		
		// FollowTime(꾹 누르고 있던 시간)을 ShortPressThreshold(Released 이벤트가 발생하지 않는 임계점)와 비교
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// 캐릭터의 현재 위치로부터 마우스가 Release된 Location까지 최단거리 계산
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				// 기존 경로 삭제
				Spline->ClearSplinePoints();
				// SplineComponent를 이용해 최단거리인 직선 경로를 곡선으로 변환 
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				
				if (NavPath->PathPoints.Num() > 0)
				{
					// 목적지 값 변경, 이 과정을 거치지 않으면 장애물을 클릭한 경우 목적지에 도달하지 못 해 영원히 달림
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					// 목적지까지 자동 달리기 시작
					bAutoRunning = true;
				}
			}
			if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
		}
		// 관련 변수 초기화
		FollowTime = 0.f;
		bTargeting = false;
	}
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	
	if (Spline->GetNumberOfSplinePoints())
	{
		Spline->ClearSplinePoints();
	}
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		// 곡선 경로 전체 중 캐릭터의 위치로부터 가장 가까운 Location을 구합니다.
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		// 구해진 해당 Location을 통해 곡선의 방향을 구합니다.
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		// 해당 방향으로 이동을 시작합니다.
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::UpdateSkillPreviewLocation() const
{
	if (SkillPreview)
	{
		if (CursorHit.GetActor() && CursorHit.GetActor()->IsA(APawn::StaticClass()))
		{
			SkillPreview->SetActorLocation(CursorHit.GetActor()->GetActorLocation());
		}
		else
		{
			SkillPreview->SetActorLocation(CursorHit.ImpactPoint);
		}
	}
}

#include "AuraCharacter.h"

#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/Player/AuraPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Aura/Player/AuraPlayerController.h"
#include "Aura/UI/HUD/AuraHUD.h"
#include "Aura/Manager/PawnManagerSubsystem.h"

AAuraCharacter::AAuraCharacter()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Ally, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Ally, ECR_Block);
}

void AAuraCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 서버의 Ability Actor 초기화
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트의 Ability Actor 초기화
	InitAbilityActorInfo();
}

int32 AAuraCharacter::GetPlayerLevel()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}

void AAuraCharacter::UnregisterPawn()
{
	UPawnManagerSubsystem* PawnManager = GetGameInstance()->GetSubsystem<UPawnManagerSubsystem>();
	PawnManager->UnregisterPlayerPawn(this);
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	// Autonomous Proxy의 AbilitySystemComponent은 Owner Actor가 PlayerState, Avatar Actor가 캐릭터
	// 이유는 리스폰 시 정보가 유지되어야 하는 상황이 존재하기 때문
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();

	// PlayerController, PlayerState, AbilitySystemComponent, AttributeSet이 모두 초기화된 게 확실한 장소이므로 HUD의 Init함수 호출
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	// 초기 Attribute 초기화
	InitializeDefaultAttributes();
}

void AAuraCharacter::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

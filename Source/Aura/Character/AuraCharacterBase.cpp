#include "AuraCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Aura/Manager/FXManagerSubsystem.h"
#include "Component/DebuffComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DebuffComponent = CreateDefaultSubobject<UDebuffComponent>("DebuffComponent");
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		RegisterPawn();
	}
}

void AAuraCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(FName SocketName, bool bFindFromWeapon)
{
	if (bFindFromWeapon && Weapon)
	{
		return Weapon->GetSocketLocation(SocketName);
	}

	if (GetMesh())
	{
		return GetMesh()->GetSocketLocation(SocketName);
	}

	return FVector::ZeroVector;
}

void AAuraCharacterBase::Die(const FVector& Impulse)
{
	if (HasAuthority())
	{
		UnregisterPawn();
	}
	MulticastDeath(Impulse);
	AbilitySystemComponent->CancelAllAbilities();
}

bool AAuraCharacterBase::IsDead_Implementation()
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

ECharacterRank AAuraCharacterBase::GetCharacterRank_Implementation()
{
	return CharacterRank;
}

FOnASCRegistered& AAuraCharacterBase::GetOnASCRegisteredDelegate()
{
	return OnASCRegistered;
}

FOnDeath& AAuraCharacterBase::GetOnDeathDelegate()
{
	return OnDeath;
}

void AAuraCharacterBase::ApplyKnockback(const FVector_NetQuantize& KnockbackForce, float Duration)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
	ConstantForce->InstanceName = FName("Knockback");
	ConstantForce->Priority = 200;
	ConstantForce->Force = KnockbackForce / Duration;
	ConstantForce->Duration = Duration;
	ConstantForce->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;
	MovementComponent->RemoveRootMotionSource(FName("Knockback"));
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}
	
	MovementComponent->ApplyRootMotionSource(ConstantForce);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		if (GetMesh() && GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
		}
	}, Duration, false);
}

USkeletalMeshComponent* AAuraCharacterBase::GetWeapon_Implementation()
{
	return Weapon;
}

void AAuraCharacterBase::MulticastPlayLoopAnimMontage_Implementation(UAnimMontage* LoopMontage)
{
	PlayAnimMontage(LoopMontage);
}

void AAuraCharacterBase::MulticastStopLoopAnimMontage_Implementation(UAnimMontage* LoopMontage)
{
	StopAnimMontage(LoopMontage);
}

void AAuraCharacterBase::MulticastDeath_Implementation(const FVector& Impulse)
{	
	if (DeathSoundTag.IsValid())
	{
		UFXManagerSubsystem* FXManager = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>();
		if (FXManager)
		{
			FXManager->AsyncPlaySoundAtLocation(DeathSoundTag, GetActorLocation());
		}
	}
	
	// SetSimulatePhysics에서 자동으로 Detach를 호출
	Weapon->SetEnableGravity(true);
	Weapon->SetSimulatePhysics(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Weapon->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Weapon->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Weapon->SetAnimationMode(EAnimationMode::Type::AnimationSingleNode);
	Weapon->AddImpulse(Impulse, NAME_None, true);
	
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->SetAnimationMode(EAnimationMode::Type::AnimationSingleNode);
	GetMesh()->AddImpulseToAllBodiesBelow(Impulse, NAME_None, true, true);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	Dissolve();

	bDead = true;
	OnDeath.Broadcast(this);
}

void AAuraCharacterBase::InitAbilityActorInfo()
{
	DebuffComponent->InitAbilityActorInfo(AbilitySystemComponent);
}

void AAuraCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AAuraCharacterBase::AddCharacterStartupAbilities() const
{
	if (!HasAuthority()) return;

	// ASC 가져와서 부여 함수 호출
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->AddCharacterAbilities(StartupAbilities);
	AuraASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

void AAuraCharacterBase::Dissolve()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateLambda([this]()
		{
			UMaterialInstanceDynamic* DynamicMatInst = nullptr;
			UMaterialInstanceDynamic* WeaponDynamicMatInst = nullptr;
			if (IsValid(DissolveMaterialInstance))
			{
				DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
				GetMesh()->SetMaterial(0, DynamicMatInst);
			}
			if (IsValid(WeaponDissolveMaterialInstance))
			{
				WeaponDynamicMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
				Weapon->SetMaterial(0, WeaponDynamicMatInst);
			}
			StartDissolveTimeline(DynamicMatInst, WeaponDynamicMatInst);
		}),
		2.f,
		false
	);
}

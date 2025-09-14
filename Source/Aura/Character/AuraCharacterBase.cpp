#include "AuraCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Aura/Manager/FXManagerSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

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
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	RegisterPawn();
}

void AAuraCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AAuraCharacterBase, bIsStunned, COND_OwnerOnly);
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
	// 서버에서만 호출되는 함수임이 명확하므로 권한 확인 필요 없이 등록 해제
	UnregisterPawn();
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
	// Effects.HitReact가 부여되었을 때에 대한 콜백 함수 바인드 구문입니다.
	// EGameplayTagEventType은 언제 호출할 건지 결정하는 enum으로,
	// NewOrRemoved는 카운트가 0에서 1로 증가하거나, 1에서 0이 될때만 호출, AnyCountChange는 카운트가 변경되면 무조건 호출됩니다.
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::HitReactTagChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Debuff_Type_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::StunTagChanged);
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

void AAuraCharacterBase::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? (BaseWalkSpeed / 2.f) : BaseWalkSpeed;
}

void AAuraCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// 기절 상태이상에 걸리면 현재 발동 중인 Active 스킬을 모두 취소합니다.
	const FAuraGameplayTags AuraGameplayTags = FAuraGameplayTags::Get();
	FGameplayTagContainer CancelAbilityTags;
	CancelAbilityTags.AddTag(AuraGameplayTags.Abilities_Types_Active);
	
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f : BaseWalkSpeed;
	AbilitySystemComponent->CancelAbilities(&CancelAbilityTags);
}

void AAuraCharacterBase::OnRep_Stunned()
{
}

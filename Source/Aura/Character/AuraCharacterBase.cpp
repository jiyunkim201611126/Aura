#include "AuraCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "Aura/Player/AuraPlayerController.h"
#include "Aura/Manager/PawnManagerSubsystem.h"

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

void AAuraCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag, const FName& SocketName)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	if (MontageTag.MatchesTagExact(GameplayTags.Montage_Attack_Weapon) && IsValid(Weapon))
	{
		return Weapon->GetSocketLocation(SocketName);
	}
	else
	{
		return GetMesh()->GetSocketLocation(SocketName);
	}
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void AAuraCharacterBase::Die(bool bShouldAddImpulse, const FVector& Impulse)
{
	// 서버에서만 호출되는 함수임이 명확하므로 권한 확인 필요 없이 등록 해제
	UnregisterPawn();
	MulticastHandleDeath(bShouldAddImpulse, Impulse);
}

bool AAuraCharacterBase::IsDead_Implementation()
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation(bool bShouldAddImpulse, const FVector& Impulse)
{
	// SetSimulatePhysics에서 자동으로 Detach를 호출
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Weapon->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (bShouldAddImpulse)
	{
		Weapon->AddImpulse(Impulse * Weapon->GetMass() * 100.f);
		GetMesh()->AddImpulse(Impulse * GetMesh()->GetMass() * 300.f);
	}

	Dissolve();

	bDead = true;
}

void AAuraCharacterBase::MulticastSpawnDamageText_Implementation(float Damage, bool bBlockedHit, bool bCriticalHit)
{
	// 리슨 서버에서도 확실하게 자신의 컨트롤러만 추적할 수 있도록 안전장치
	for (TActorIterator<AAuraPlayerController> It(GetWorld()); It; ++It)
	{
		if (It->IsLocalController())
		{
			It->SpawnDamageText(Damage, this, bBlockedHit, bCriticalHit);
			break;
		}
	}
}

void AAuraCharacterBase::InitAbilityActorInfo()
{
}

void AAuraCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AAuraCharacterBase::AddCharacterStartupAbilities() const
{
	if (!HasAuthority()) return;

	// ASC 가져와서 장착 함수 호출
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->AddCharacterAbilities(StartupAbilities);
}

void AAuraCharacterBase::Dissolve()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
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

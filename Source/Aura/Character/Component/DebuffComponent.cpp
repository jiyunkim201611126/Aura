#include "DebuffComponent.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/Niagara/AuraNiagaraComponent.h"
#include "Aura/AI/AuraAIController.h"
#include "Aura/Character/AuraEnemy.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UDebuffComponent::UDebuffComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDebuffComponent::InitAbilityActorInfo(UAbilitySystemComponent* InAbilitySystemComponent)
{
	// Effects.HitReact가 부여되었을 때에 대한 콜백 함수 바인드 구문입니다.
	// EGameplayTagEventType은 언제 호출할 건지 결정하는 enum으로,
	// NewOrRemoved는 카운트가 0에서 1로 증가하거나, 1에서 0이 될때만 호출, AnyCountChange는 카운트가 변경되면 무조건 호출됩니다.
	AbilitySystemComponent = InAbilitySystemComponent;
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::HitReactTagChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Debuff_Types_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::StunTagChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Debuff_Types_Burn, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::BurnTagChanged);
}

void UDebuffComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UDebuffComponent, bIsStunned, COND_OwnerOnly);
}

void UDebuffComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AAuraEnemy* Enemy = GetPawn<AAuraEnemy>())
	{
		AuraAIController = Enemy->AuraAIController;
	}
}

UAuraNiagaraComponent* UDebuffComponent::CreateNiagaraComponent(const FGameplayTag& DebuffTypeTag)
{
	// 이펙트 적용 시 GrantedTag가 자동으로 부여되므로, 그 전에 이미 해당 디버프가 부여되어있는지 확인해 나이아가라가 중복으로 생기지 않도록 방지합니다.
	if (UAuraNiagaraComponent* NiagaraComponent = NewObject<UAuraNiagaraComponent>(this))
	{
		NiagaraComponent->NiagaraTag = DebuffTypeTag;
		NiagaraComponent->SetupAttachment(GetPawn<ACharacter>()->GetMesh(), FName("RootSocket"));
		NiagaraComponent->RegisterComponent();
		return NiagaraComponent;
	}
	
	return nullptr;
}

void UDebuffComponent::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;

	// 플레이어 캐릭터의 동작입니다.
	if (const AAuraCharacterBase* OwnerCharacter = GetPawn<AAuraCharacterBase>())
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? (OwnerCharacter->BaseWalkSpeed / 2.f) : OwnerCharacter->BaseWalkSpeed;
	}

	// AI 캐릭터의 동작입니다.
	if (AuraAIController.IsValid() && AuraAIController->GetBlackboardComponent())
	{
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void UDebuffComponent::BurnTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		if (UAuraNiagaraComponent* NiagaraComponent = CreateNiagaraComponent(CallbackTag))
		{
			if (const ACharacter* Character = GetPawn<ACharacter>())
			{
				const float CharacterHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				NiagaraComponent->SetRelativeLocation(FVector(0.f, 0.f, CharacterHeight));
			}
		}
	}
}

void UDebuffComponent::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	if (bIsStunned)
	{
		if (UAuraNiagaraComponent* NiagaraComponent = CreateNiagaraComponent(CallbackTag))
		{
			if (const ACharacter* Character = GetPawn<ACharacter>())
			{
				const float CharacterHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				const float CharacterWidth = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
				NiagaraComponent->SetRelativeLocation(FVector(0.f, 0.f, CharacterHeight * 2.f));
				NiagaraComponent->SetRelativeScale3D(FVector(CharacterWidth / 36.f));
			}
		}
	}
	
	// 기절 상태이상에 걸리면 현재 발동 중인 Active 스킬을 모두 취소합니다.
	const FAuraGameplayTags AuraGameplayTags = FAuraGameplayTags::Get();
	FGameplayTagContainer CancelAbilityTags;
	CancelAbilityTags.AddTag(AuraGameplayTags.Abilities_Types_Active);
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->CancelAbilities(&CancelAbilityTags);
	}

	// 플레이어 캐릭터의 동작입니다.
	if (const AAuraCharacterBase* OwnerCharacter = GetPawn<AAuraCharacterBase>())
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f : OwnerCharacter->BaseWalkSpeed;
		SetBlockInputState();
		return;
	}

	// AI 캐릭터의 동작입니다.
	if (AuraAIController.IsValid() && AuraAIController->GetBlackboardComponent())
	{
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bIsStunned);
	}
}

void UDebuffComponent::SetBlockInputState() const
{
	// 자신의 캐릭터가 아닌 경우 즉시 return합니다.
	if (!GetPawn<APawn>() || !GetPawn<APawn>()->IsLocallyControlled())
	{
		return;
	}
	
	// 클라이언트측에서 명시적으로 BlockedTags를 부여해주지 않으면, 기절 상태이상 중에 움직이려 시도하면서 러버밴딩 현상이 발생합니다.
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		FGameplayTagContainer BlockedTags;
		BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);
		bIsStunned ? AuraASC->AddLooseGameplayTags(BlockedTags) : AuraASC->RemoveLooseGameplayTags(BlockedTags);
	}
}


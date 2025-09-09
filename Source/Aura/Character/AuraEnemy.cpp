#include "AuraEnemy.h"

#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "Aura/UI/Widget/AuraUserWidget.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Aura/AI/AuraAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Aura/Manager/PawnManagerSubsystem.h"
#include "Aura/UI/Widget/ProgressBar/AuraProgressBar.h"

AAuraEnemy::AAuraEnemy()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Enemy, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Enemy, ECR_Block);

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	// Simulated Proxy는 최소한의 정보만 복제되도록 설정
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
	HealthBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AAuraEnemy::HighlightActor()
{
	GetMesh()->SetRenderCustomDepth(true);
	Weapon->SetRenderCustomDepth(true);
}

void AAuraEnemy::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AAuraEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AAuraEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void AAuraEnemy::RegisterPawn()
{
	if (HasAuthority())
	{
		if (UPawnManagerSubsystem* PawnManager = GetGameInstance()->GetSubsystem<UPawnManagerSubsystem>())
		{
			PawnManager->RegisterAIPawn(this);
		}
	}
}

void AAuraEnemy::UnregisterPawn()
{
	if (HasAuthority())
	{
		if (UPawnManagerSubsystem* PawnManager = GetGameInstance()->GetSubsystem<UPawnManagerSubsystem>())
		{
			PawnManager->UnregisterAIPawn(this);
		}
	}
}

int32 AAuraEnemy::GetCharacterLevel_Implementation()
{
	return Level;
}

void AAuraEnemy::Die(const FVector& Impulse)
{
	SetLifeSpan(LifeSpan);
	if (AuraAIController)
	{
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
	}
	
	Super::Die(Impulse);
}

void AAuraEnemy::MulticastDeath_Implementation(const FVector& Impulse)
{
	// 하수인의 경우, 주인 사망 시 Damage에 의해 사망하는 게 아닌 기믹에 의한 사망이므로 체력바가 반영되지 않는 경우가 있습니다.
	// 따라서 사망 시 강제로 체력바를 0%로 표시합니다.
	if (HealthBar->GetWidget())
	{
		Cast<UAuraProgressBar>(HealthBar->GetWidget())->SetBarPercent_Implementation(0, 1);
	}
	Super::MulticastDeath_Implementation(Impulse);
}

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	
	InitAbilityActorInfo();
	if (HasAuthority())
	{
		UAuraAbilitySystemLibrary::AddCommonAbilities(this, AbilitySystemComponent, CharacterClass);
		AddCharacterStartupAbilities();
	}

	// WidgetController가 UObject로 선언되어있으므로 Character가 직접 Controller가 되는 것도 가능
	if (UAuraUserWidget* AuraUserWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		AuraUserWidget->SetWidgetController(this);
	}
	
	// WidgetComponent를 위한 델리게이트 바인드
	if (const UAuraAttributeSet* AuraAS = Cast<UAuraAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			});
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			});

		// Effects.HitReact가 부여되었을 때에 대한 콜백 함수 바인드, EGameplayTagEventType은 언제 호출할 건지 결정하는 enum
		// NewOrRemoved는 카운트가 0에서 1로 증가하거나, 1에서 0이 될때만 호출, AnyCountChange는 카운트가 변경되면 무조건 호출 
		AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraEnemy::HitReactTagChanged);
		
		OnHealthChanged.Broadcast(AuraAS->GetHealth());
		OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	}
}

void AAuraEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// AIController는 어떻게 행동할지 '판단'하는 클래스
	// 클라이언트는 판단할 필요가 없으니 바로 return
	if (!HasAuthority())
	{
		return;
	}
	AuraAIController = Cast<AAuraAIController>(NewController);
	AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	// 아래 값 할당 함수들은 반드시 RunBehaviorTree 이후에 호출합니다.
	AuraAIController->RunBehaviorTree(BehaviorTree);
	
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	AuraAIController->GetBlackboardComponent()->SetValueAsFloat(FName("AgroRange"), AgroRange);
	AuraAIController->GetBlackboardComponent()->SetValueAsFloat(FName("CombatRange"), CombatRange);
	
	if (AgroBehaviorTree)
	{
		AuraAIController->BehaviorTreeComponent->SetDynamicSubtree(FAuraGameplayTags::Get().BT_Sub_Agro, AgroBehaviorTree);
	}
	if (CombatBehaviorTree)
	{
		AuraAIController->BehaviorTreeComponent->SetDynamicSubtree(FAuraGameplayTags::Get().BT_Sub_Combat, CombatBehaviorTree);
	}
}

void AAuraEnemy::InitAbilityActorInfo()
{
	// Simulated Proxy의 AbilitySystemComponent는 Owner Actor == Avatar Actor로, 둘 모두 캐릭터입니다.
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	if (HasAuthority())
	{
		UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
	}
	OnASCRegistered.Broadcast(AbilitySystemComponent);
}

void AAuraEnemy::AddCharacterStartupAbilities() const
{
	if (!HasAuthority()) return;

	// ASC 가져와서 부여 함수 호출
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->AddAbilities(StartupAbilities);
	AuraASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

void AAuraEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	if (AuraAIController && AuraAIController->GetBlackboardComponent())
	{
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

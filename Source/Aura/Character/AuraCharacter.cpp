#include "AuraCharacter.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/AbilitySystem/Data/AbilityInfo.h"
#include "Aura/Game/SaveGame/AuraSaveGame.h"
#include "Aura/Player/AuraPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Aura/Player/AuraPlayerController.h"
#include "Aura/UI/HUD/AuraHUD.h"
#include "Aura/Manager/PawnManagerSubsystem.h"
#include "Aura/Manager/FXManagerSubsystem.h"
#include "Aura/Manager/SaveManagerSubsystem.h"

AAuraCharacter::AAuraCharacter()
{
	CharacterClass = ECharacterClass::Elementalist;
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

	// PlayerState, PlayerController, 빙의할 Pawn(this)의 생성이 확실한 서버의 타이밍입니다.
	// 따라서 이곳에서 InitAbilityActorInfo를 호출합니다.
	InitAbilityActorInfo();
	LoadProgress();
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// PlayerState, PlayerController, 빙의할 Pawn(this)의 생성이 확실한 클라이언트의 타이밍입니다.
	// 따라서 이곳에서 InitAbilityActorInfo를 호출합니다.
	InitAbilityActorInfo();
}

void AAuraCharacter::RegisterPawn()
{
	if (HasAuthority())
	{
		if (UPawnManagerSubsystem* PawnManager = GetGameInstance()->GetSubsystem<UPawnManagerSubsystem>())
		{
			PawnManager->RegisterPlayerPawn(this);
		}
	}
}

void AAuraCharacter::UnregisterPawn()
{
	if (HasAuthority())
	{
		if (UPawnManagerSubsystem* PawnManager = GetGameInstance()->GetSubsystem<UPawnManagerSubsystem>())
		{
			PawnManager->UnregisterPlayerPawn(this);
		}
	}
}

int32 AAuraCharacter::GetCharacterLevel_Implementation()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}

int32 AAuraCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}

int32 AAuraCharacter::GetXP_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetXP();
}

int32 AAuraCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->LevelUpInformation[Level].AttributePointAward;
}

int32 AAuraCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->LevelUpInformation[Level].SpellPointAward;
}

void AAuraCharacter::AddToXP_Implementation(const int32 InXP)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToXP(InXP);
}

void AAuraCharacter::AddToLevel_Implementation(const int32 InPlayerLevel)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToLevel(InPlayerLevel);

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		// 레벨이 상승했으므로, Abilities 목록을 업데이트합니다.
		AuraASC->UpdateAbilityStatuses(AuraPlayerState->GetPlayerLevel());
	}
}

void AAuraCharacter::AddToAttributePoints_Implementation(const int32 InAttributePoints)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToAttributePoints(InAttributePoints);
}

int32 AAuraCharacter::GetAttributePoints_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetAttributePoints();
}

int32 AAuraCharacter::GetSpellPoints_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetSpellPoints();
}

void AAuraCharacter::AddToSpellPoints_Implementation(const int32 InSpellPoints)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToSpellPoints(InSpellPoints);
}

void AAuraCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles();
}

void AAuraCharacter::SaveProgress_Implementation(const FName& CheckpointTag)
{
	USaveManagerSubsystem* SaveManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	if (SaveManagerSubsystem)
	{
		UAuraSaveGame* SaveData = SaveManagerSubsystem->RetrieveInGameSaveData();
		if (SaveData)
		{
			SaveData->PlayerStartTag = CheckpointTag;

			if (AAuraPlayerState* AuraPlayerState = Cast<AAuraPlayerState>(GetPlayerState()))
			{
				SaveData->PlayerLevel = AuraPlayerState->GetPlayerLevel();
				SaveData->XP = AuraPlayerState->GetXP();
				SaveData->SpellPoints = AuraPlayerState->GetSpellPoints();
				SaveData->AttributePoints = AuraPlayerState->GetAttributePoints();
			}
			SaveData->Strength = UAuraAttributeSet::GetStrengthAttribute().GetNumericValue(GetAttributeSet());
			SaveData->Intelligence = UAuraAttributeSet::GetIntelligenceAttribute().GetNumericValue(GetAttributeSet());
			SaveData->Resilience = UAuraAttributeSet::GetResilienceAttribute().GetNumericValue(GetAttributeSet());
			SaveData->Vigor = UAuraAttributeSet::GetVigorAttribute().GetNumericValue(GetAttributeSet());

			UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent());
			
			for (const FGameplayAbilitySpec& AbilitySpec : AuraASC->GetActivatableAbilities())
			{
				const FGameplayTag AbilityTag = AuraASC->GetAbilityTagFromSpec(AbilitySpec);
				const FGameplayTag StatusTag = AuraASC->GetStatusFromSpec(AbilitySpec);
				const FGameplayTag InputTag = AuraASC->GetInputTagFromAbilityTag(AbilityTag);
				
				FSavedAbility NewSavedAbility;
				NewSavedAbility.AbilityTag = AbilityTag;
				NewSavedAbility.AbilityStatus = StatusTag;
				NewSavedAbility.InputTag = InputTag;
				NewSavedAbility.AbilityLevel = AbilitySpec.Level;

				// 저장된 Abilities 데이터 내에 동일한 Ability가 존재하는지 탐색합니다.
				// 해당 로직은 오버로드된 연산자를 통해 수행됩니다.
				const int32 SavedAbilityIndex = SaveData->SavedAbilities.Find(NewSavedAbility);
				if (SavedAbilityIndex != INDEX_NONE)
				{
					// 동일한 Ability가 있는 경우 들어오는 분기입니다.
					SaveData->SavedAbilities[SavedAbilityIndex] = NewSavedAbility;
				}
				else
				{
					// 동일한 Ability가 존재하지 않는 경우 들어오는 분기입니다.
					SaveData->SavedAbilities.Add(NewSavedAbility);
				}
			}

			SaveData->bFirstTimeLoadIn = false;
			SaveManagerSubsystem->SaveInGameProgressData(SaveData);
		}
	}
}

void AAuraCharacter::LoadProgress()
{
	USaveManagerSubsystem* SaveManagerSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	if (SaveManagerSubsystem)
	{
		UAuraSaveGame* SaveData = SaveManagerSubsystem->RetrieveInGameSaveData();
		if (SaveData)
		{
			if (AAuraPlayerState* AuraPlayerState = Cast<AAuraPlayerState>(GetPlayerState()))
			{
				AuraPlayerState->SetPlayerLevel(SaveData->PlayerLevel);
				AuraPlayerState->SetXP(SaveData->XP);
				AuraPlayerState->SetSpellPoints(SaveData->SpellPoints);
				AuraPlayerState->SetAttributePoints(SaveData->AttributePoints);
			}
			
			if (SaveData->bFirstTimeLoadIn)
			{
				// 캐릭터 첫 생성 시 들어오는 분기입니다.
				InitializeDefaultAttributes();
				AddCharacterStartupAbilities();
			}
			else
			{
				// 저장된 데이터를 불러올 때 들어오는 분기입니다.
				if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent()))
				{
					AuraASC->AddCharacterAbilitiesFromSaveData(SaveData);
				}
				
				UAuraAbilitySystemLibrary::InitializeAttributesFromSaveData(this, AbilitySystemComponent, SaveData);
			}
		}
	}
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	// Autonomous Proxy의 AbilitySystemComponent은 Owner Actor가 PlayerState, Avatar Actor가 캐릭터
	// 이유는 리스폰 시 정보가 유지되어야 하는 상황이 존재하기 때문입니다.
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
	OnASCRegistered.Broadcast(AbilitySystemComponent);

	// PlayerController, PlayerState, AbilitySystemComponent, AttributeSet이 모두 초기화된 게 확실한 장소이므로 HUD의 Init함수를 호출합니다.
	// Widget들의 생성 자체는 이미 끝난 상태이며, 콜백 함수 바인드 로직의 호출을 기다리고 있는 상태입니다.
	// InitHUD 혹은 라이브러리 함수에 의해 Widget Controller 객체가 생성됩니다.
	// Widget Controller는 Widget과 관련 있는 객체들을 직접 찾아가 자신의 콜백 함수를 바인드하고, 그 과정이 끝나면 모든 Widget들에게 Widget Controller가 뿌려집니다.
	if (AAuraPlayerController* AuraPlayerController = GetController<AAuraPlayerController>())
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitHUD(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	
	Super::InitAbilityActorInfo();
}

void AAuraCharacter::MulticastLevelUpParticles_Implementation() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFXManagerSubsystem* FXManager = GameInstance->GetSubsystem<UFXManagerSubsystem>())
		{
			FXManager->AsyncSpawnNiagaraAtLocation(LevelUpNiagaraTag, GetActorLocation());
		}
	}
}

void AAuraCharacter::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

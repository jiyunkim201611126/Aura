#include "AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Aura/UI/HUD/AuraHUD.h"
#include "Aura/Player/AuraPlayerState.h"
#include "Aura/Game/AuraGameModeBase.h"
#include "Aura/AuraAbilityTypes.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "Aura/UI/WidgetController/AuraWidgetController.h"
#include "Data/EliminateRewardInfo.h"

bool UAuraAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWidgetControllerParams, AAuraHUD*& OutAuraHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		UAttributeSet* AS = PS->GetAttributeSet();
		
		OutWidgetControllerParams.PlayerController = PC;
		OutWidgetControllerParams.PlayerState = PS;
		OutWidgetControllerParams.AbilitySystemComponent = ASC;
		OutWidgetControllerParams.AttributeSet = AS;
		OutAuraHUD = Cast<AAuraHUD>(PC->GetHUD());
		
		return true;
	}
	return false;
}

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WidgetControllerParams;
	AAuraHUD* AuraHUD = nullptr;
	const bool bSuccessfulParams = MakeWidgetControllerParams(WorldContextObject, WidgetControllerParams, AuraHUD);

	if (bSuccessfulParams)
	{
		return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);
	}

	return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WidgetControllerParams;
	AAuraHUD* AuraHUD = nullptr;
	const bool bSuccessfulParams = MakeWidgetControllerParams(WorldContextObject, WidgetControllerParams, AuraHUD);

	if (bSuccessfulParams)
	{
		return AuraHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
	}

	return nullptr;
}

USpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WidgetControllerParams;
	AAuraHUD* AuraHUD = nullptr;
	const bool bSuccessfulParams = MakeWidgetControllerParams(WorldContextObject, WidgetControllerParams, AuraHUD);

	if (bSuccessfulParams)
	{
		return AuraHUD->GetSpellMenuWidgetController(WidgetControllerParams);
	}

	return nullptr;
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	const FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	
	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttribute, Level, PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());
	
	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttribute, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());
	
	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttribute, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

void UAuraAbilitySystemLibrary::AddCommonAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr)
	{
		return;
	}

	// 모든 ASC가 가지는 공통 Ability 부여
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}

	// CharacterClass별로 상이한 Ability 부여
	const FCharacterClassDefaultInfo DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetCharacterLevel(ASC->GetAvatarActor()));
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr)
	{
		return nullptr;
	}
	
	return AuraGameMode->CharacterClassInfo;
}

UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr)
	{
		return nullptr;
	}

	return AuraGameMode->AbilityInfo;
}

FDamageDataContext UAuraAbilitySystemLibrary::GetDamageData(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDamageData();
	}
	return FDamageDataContext();
}

FDebuffDataContext UAuraAbilitySystemLibrary::GetDebuffData(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDebuffData();
	}
	return FDebuffDataContext();
}

TArray<FVector_NetQuantize> UAuraAbilitySystemLibrary::GetLocationsFromContext( const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetLocations();
	}
	return TArray<FVector_NetQuantize>();
}

void UAuraAbilitySystemLibrary::SetDamageDataContext(FGameplayEffectContextHandle& EffectContextHandle, const EDamageTypeContext DamageType, bool bIsBlocked, bool bIsCritical)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDamageDataContext(DamageType, bIsBlocked, bIsCritical);
	}
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle, const FVector& Impulse)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDeathImpulse(Impulse);
	}
}

void UAuraAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle, const FVector& Force)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetKnockbackForce(Force);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDataContext(FGameplayEffectContextHandle& EffectContextHandle, const FDebuffDataContext& Data)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffDataContext(Data);
	}
}

void UAuraAbilitySystemLibrary::SetLocationsToContext(FGameplayEffectContextHandle& EffectContextHandle, const TArray<FVector_NetQuantize>& InLocations)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetLocations(InLocations);
	}
}

EDamageTypeContext UAuraAbilitySystemLibrary::ReplaceDamageTypeToEnum(const FGameplayTag& DamageTypeTag)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	if (DamageTypeTag == GameplayTags.Damage_Fire)
	{
		return EDamageTypeContext::Fire;
	}
	if (DamageTypeTag == GameplayTags.Damage_Lightning)
	{
		return EDamageTypeContext::Lightning;
	}
	if (DamageTypeTag == GameplayTags.Damage_Arcane)
	{
		return EDamageTypeContext::Arcane;
	}
	if (DamageTypeTag == GameplayTags.Damage_Physical)
	{
		return EDamageTypeContext::Physical;
	}
	
	return EDamageTypeContext::None;
}

FGameplayTag UAuraAbilitySystemLibrary::ReplaceDamageTypeToTag(const EDamageTypeContext DamageTypeEnum)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	if (DamageTypeEnum == EDamageTypeContext::Fire)
	{
		return GameplayTags.Damage_Fire;
	}
	if (DamageTypeEnum == EDamageTypeContext::Lightning)
	{
		return GameplayTags.Damage_Lightning;
	}
	if (DamageTypeEnum == EDamageTypeContext::Arcane)
	{
		return GameplayTags.Damage_Arcane;
	}
	if (DamageTypeEnum == EDamageTypeContext::Physical)
	{
		return GameplayTags.Damage_Physical;
	}

	return FGameplayTag();
}

EDebuffTypeContext UAuraAbilitySystemLibrary::ReplaceDebuffTypeToEnum(const FGameplayTag& DebuffTypeTag)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	if (DebuffTypeTag == GameplayTags.Debuff_Type_Burn)
	{
		return EDebuffTypeContext::Burn;
	}
	if (DebuffTypeTag == GameplayTags.Debuff_Type_Stun)
	{
		return EDebuffTypeContext::Stun;
	}
	if (DebuffTypeTag == GameplayTags.Debuff_Type_Confuse)
	{
		return EDebuffTypeContext::Confuse;
	}
	
	return EDebuffTypeContext::None;
}

FGameplayTag UAuraAbilitySystemLibrary::ReplaceDebuffTypeToTag(const EDebuffTypeContext DebuffTypeEnum)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	if (DebuffTypeEnum == EDebuffTypeContext::Burn)
	{
		return GameplayTags.Debuff_Type_Burn;
	}
	if (DebuffTypeEnum == EDebuffTypeContext::Stun)
	{
		return GameplayTags.Debuff_Type_Stun;
	}
	if (DebuffTypeEnum == EDebuffTypeContext::Confuse)
	{
		return GameplayTags.Debuff_Type_Confuse;
	}
	
	return FGameplayTag();
}

TArray<FVector> UAuraAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, const float Spread, const int32 NumOfVectors)
{
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	const float DeltaSpread = NumOfVectors > 1 ? Spread / (NumOfVectors - 1) : 0.f;

	TArray<FVector> ResultVectors;
	ResultVectors.Reserve(NumOfVectors);
	for (int i = 0; i < NumOfVectors; i++)
	{
		const FVector Direction = NumOfVectors > 1 ? LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector) : Forward;
		ResultVectors.Add(Direction);
	}

	return ResultVectors;
}

TArray<FRotator> UAuraAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, const float Spread, const int32 NumOfRotators)
{
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	const float DeltaSpread = NumOfRotators > 1 ? Spread / (NumOfRotators - 1) : 0.f;

	TArray<FRotator> ResultRotators;
	ResultRotators.Reserve(NumOfRotators);
	for (int i = 0; i < NumOfRotators; i++)
	{
		const FVector Direction = NumOfRotators > 1 ? LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector) : Forward;
		ResultRotators.Add(Direction.Rotation());
	}

	return ResultRotators;
}

void UAuraAbilitySystemLibrary::GetOverlappedLivePawnsWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	// 충돌 검사 조건 세부 설정하는 구조체
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	TArray<FOverlapResult> Overlaps;
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}

void UAuraAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	if (Actors.Num() < MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}
	
	for (const auto Actor : Actors)
	{
		// 아직 MaxTargets 수를 다 채우지 못 했다면 바로 추가합니다.
		if (OutClosestTargets.Num() < MaxTargets)
		{
			OutClosestTargets.AddUnique(Actor);
			continue;
		}
		
		// 가장 먼 타겟을 탐색합니다.
		int32 FarthestActorIndex = 0;
		float MaxDistance = FVector::DistSquared(Origin, OutClosestTargets[0]->GetActorLocation());
		for (int32 i = 1; i < OutClosestTargets.Num(); i++)
		{
			const float CompareDistance = FVector::DistSquared(Origin, OutClosestTargets[i]->GetActorLocation());
			if (CompareDistance > MaxDistance)
			{
				MaxDistance = CompareDistance;
				FarthestActorIndex = i;
			}
		}

		// 가장 먼 타겟의 거리보다 이번 타겟의 거리가 가까우면 교체합니다.
		const float CurrentActorDistance = FVector::DistSquared(Origin, Actor->GetActorLocation());
		if (CurrentActorDistance < MaxDistance)
		{
			OutClosestTargets[FarthestActorIndex] = Actor;
		}
	}
}

bool UAuraAbilitySystemLibrary::IsFriend(const AActor* FirstActor, const AActor* SecondActor)
{
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
	const bool bFriends = bBothArePlayers || bBothAreEnemies;
	return bFriends;
}

TArray<AActor*> UAuraAbilitySystemLibrary::GetActorsFromContext(FGameplayEffectContextHandle& EffectContextHandle)
{
	TArray<AActor*> Actors;
	for (auto Element : EffectContextHandle.GetActors())
	{
		Actors.Add(Element.Get());
	}
	return Actors;
}

float UAuraAbilitySystemLibrary::GetRemainingTimeByTag(UAbilitySystemComponent* ASC, FGameplayTag Tag)
{
	if (!ASC)
	{
		return 0.0f;
	}

	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAllOwningTags(Tag.GetSingleTagContainer());
	TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(Query);

	float MaxTime = 0.f;
	for (float Time : TimesRemaining)
	{
		if (Time > MaxTime)
		{
			MaxTime = Time;
		}
	}
	
	return MaxTime;
}

int32 UAuraAbilitySystemLibrary::GetXPRewardForRankAndLevel(const UObject* WorldContextObject, ECharacterRank CharacterRank, int32 CharacterLevel)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr)
	{
		return 0;
	}

	const FEliminateRewardDefaultInfo Info = AuraGameMode->EliminateRewardInfo->GetEliminateRewardInfoByRank(CharacterRank);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return XPReward;
}

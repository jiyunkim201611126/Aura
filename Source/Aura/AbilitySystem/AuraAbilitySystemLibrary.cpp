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

void UAuraAbilitySystemLibrary::SetDamageDataContext(FGameplayEffectContextHandle& EffectContextHandle, const EDamageTypeContext DamageType, bool bIsBlocked, bool bIsCritical)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		FDamageDataContext DamageData = GetDamageData(EffectContextHandle);
		DamageData.DamageType = DamageType;
		DamageData.bIsBlockedHit = bIsBlocked;
		DamageData.bIsCriticalHit = bIsCritical;
		AuraEffectContext->SetDamageDataContext(DamageData);
	}
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle, const FVector& Impulse)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		FDamageDataContext DamageData = GetDamageData(EffectContextHandle);
		DamageData.DeathImpulse = Impulse;
		AuraEffectContext->SetDamageDataContext(DamageData);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDataContext(FGameplayEffectContextHandle& EffectContextHandle, const FDebuffDataContext& Data)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffDataContext(Data);
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
	
	if (DebuffTypeTag == GameplayTags.Debuff_Burn)
	{
		return EDebuffTypeContext::Burn;
	}
	if (DebuffTypeTag == GameplayTags.Debuff_Stun)
	{
		return EDebuffTypeContext::Stun;
	}
	if (DebuffTypeTag == GameplayTags.Debuff_Confuse)
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
		return GameplayTags.Debuff_Burn;
	}
	if (DebuffTypeEnum == EDebuffTypeContext::Stun)
	{
		return GameplayTags.Debuff_Stun;
	}
	if (DebuffTypeEnum == EDebuffTypeContext::Confuse)
	{
		return GameplayTags.Debuff_Confuse;
	}
	
	return FGameplayTag();
}

void UAuraAbilitySystemLibrary::GetOverlappedLivePawns(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	// 충돌 검사 조건 세부 설정하는 구조체
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	TArray<FOverlapResult> Overlaps;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
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

#include "AuraAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "Aura/Interaction/CombatInterface.h"
#include "AuraAbilitySystemLibrary.h"
#include "EngineUtils.h"
#include "Aura/AuraAbilityTypes.h"
#include "Aura/Interaction/LevelableInterface.h"
#include "Aura/Player/AuraPlayerController.h"
#include "Debuff/DebuffNiagaraComponent.h"
#include "GameFramework/Character.h"

UAuraAttributeSet::UAuraAttributeSet()
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	// Primary Attributes

	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Resilience, GetResilienceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Vigor, GetVigorAttribute);

	// Secondary Attributes

	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_Armor, GetArmorAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, GetArmorPenetrationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_BlockChance, GetBlockChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, GetCriticalHitChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, GetCriticalHitDamageAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, GetCriticalHitResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_HealthRegeneration, GetHealthRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ManaRegeneration, GetManaRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMana, GetMaxManaAttribute);
	
	// Resistance Attributes

	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Fire, GetFireResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Lightning, GetLightningResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Arcane, GetArcaneResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Physical, GetPhysicalResistanceAttribute);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Vital Attributes

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	
	// Primary Attributes
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	// Secondary Attributes
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	// Resistance Attributes
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	// Effect 적용 전에 사망 여부를 체크해서 적용 자체를 안 하는 게 네트워크 면에서 더 좋습니다.
	// 하지만 지속 데미지 디버프 같은 상황에선 이미 Effect가 적용되어있기도 하고, 또 혹시 모를 경우를 대비해 걸러줍니다.
	if (Props.TargetCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(Props.TargetCharacter))
	{
		return;
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}

	// IncomingDamage Attribute에 값 변화가 있는 경우 데미지 로직을 시작합니다.
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		ApplyIncomingDamage(Props, Data);
	}

	// IncomingXP Attribute에 값 변화가 있는 경우 경험치 증가 및 레벨업 로직 시작합니다.
	// 위에서 SendXPEvent를 호출, GA_ListenForEvents가 해당 이벤트를 수신해 GE를 적용한 뒤 다시 여기로 들어와 아래 로직을 실행하는 방식입니다.
	if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
	{
		ApplyIncomingXP(Props);
	}
	
	if (Data.EvaluatedData.Attribute == GetIncomingDebuffAttribute())
	{
		ApplyDebuff(Props);
	}
}

void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute() && bTopOffHealth)
	{
		SetHealth(GetMaxHealth());
		bTopOffHealth = false;
	}
	if (Attribute == GetHealthAttribute() && bTopOffMana)
	{
		SetMana(GetMaxMana());
		bTopOffMana = false;
	}
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	// Source = Effect를 발생시킨 것, Target = Effect를 받는 것 (AbilitySystemComponent 소유자)
	// GE가 적용되는 상황에서 ASC, SourceAvatarActor, TargetAvatarActor등을 편리하게 추적하기 위해 사용하는 함수입니다. 
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void UAuraAttributeSet::SendXPEvent(const FEffectProperties& Props) const
{
	const ECharacterRank TargetRank = ICombatInterface::Execute_GetCharacterRank(Props.TargetAvatarActor);
	// Rank가 None인 경우(소환된 하수인이거나, 경험치를 얻을 수 없는 적), 이벤트를 발생시키지 않습니다.
	if (TargetRank == ECharacterRank::None)
	{
		return;
	}
	
	// XP 변화량을 계산해 이벤트를 송신합니다.
	const int32 TargetLevel = ICombatInterface::Execute_GetCharacterLevel(Props.TargetAvatarActor);
	const int32 XPReward = UAuraAbilitySystemLibrary::GetXPRewardForRankAndLevel(Props.TargetAvatarActor, TargetRank, TargetLevel);

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	FGameplayEventData Payload;
	Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;
	Payload.EventMagnitude = XPReward;
	// 이벤트를 송신하는 함수입니다.
	// 아래 구문을 기준으로 하면, SourceCharacter의 ASC에게 IncomingXP Tag를 식별자로 하여 이벤트를 발생시킵니다.
	// Payload에 원하는 데이터를 담아 송신할 수 있습니다.
	// 해당 이벤트는 이 Tag를 기준으로 WaitGameplayEvent를 호출한 Ability(GA_ListenForEvent)가 수신합니다.
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, GameplayTags.Attributes_Meta_IncomingXP, Payload);
}

void UAuraAttributeSet::ApplyIncomingDamage(const FEffectProperties& Props, const FGameplayEffectModCallbackData& Data)
{
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage(0.f);
	if (LocalIncomingDamage > 0.f)
	{
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

		const bool bFatal = NewHealth <= 0.f;

		const FDamageDataContext DamageData = UAuraAbilitySystemLibrary::GetDamageData(Props.EffectContextHandle);
		
		if (bFatal)
		{
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor))
			{
				CombatInterface->Die(DamageData.DeathImpulse);
			}

			// 적을 처치했으므로, XP 이벤트를 송신합니다.
			SendXPEvent(Props);
		}
		else
		{
			// 사망하지 않은 경우, 적용된 GE가 GrantHitReact 태그를 갖고 있으면 HitReact Ability를 작동합니다.
			// GrantedTag로 사용하는 게 적절하지만, GE_Damage의 Duration Policy는 Instant기 때문에 GrantedTag를 가질 수 없어 AssetTag를 사용합니다.
			FGameplayTagContainer EffectTags;
			Data.EffectSpec.GetAllAssetTags(EffectTags);
			if (EffectTags.HasTag(FAuraGameplayTags::Get().Effects_GrantHitReact))
			{
				FGameplayTagContainer HitReactTag;
				HitReactTag.AddTag(FAuraGameplayTags::Get().Abilities_HitReact);
				Props.TargetASC->TryActivateAbilitiesByTag(HitReactTag);
			}

			if (!DamageData.KnockbackForce.IsNearlyZero(1.f))
			{
				if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor))
				{
					CombatInterface->ApplyKnockback(DamageData.KnockbackForce, 0.5f);
				}
			}
		}

		// 데미지를 Text로 표시하는 위젯 컴포넌트를 AttributeSet이 직접 스폰하려면 그 클래스를 참조하고 있어야 합니다.
		// 즉, 클라이언트당 하나만 있어도 되는 포인터가 AttributeSet마다 하나씩 있게 되기 때문에 메모리가 낭비됩니다.
		// 따라서 클라이언트당 하나만 있는 PlayerController를 통해 스폰시켜주는 편이 메모리면에서 이득입니다.
		// 다만 NetMulticast 함수는 '그 액터가 클라이언트에 존재할 때'만 호출되므로, PlayerController를 순회하며 Client함수를 호출합니다.
		for (TActorIterator<AAuraPlayerController> It(GetWorld()); It; ++It)
		{
			It->ClientSpawnDamageText(LocalIncomingDamage, Props.TargetAvatarActor, DamageData.bIsBlockedHit, DamageData.bIsCriticalHit, DamageData.DamageType);
		}
	}
	else if (LocalIncomingDamage < 0.01f)
	{
		// 데미지가 0.01보다 작으면 체력 감소나 애니메이션 재생 없이 NoDamage 문구를 표시합니다.
		SetIncomingDamage(0.f);
		for (TActorIterator<AAuraPlayerController> It(GetWorld()); It; ++It)
		{
			It->ClientSpawnDamageText(LocalIncomingDamage, Props.TargetAvatarActor, false, false, EDamageTypeContext::None);
		}
	}
}

void UAuraAttributeSet::ApplyIncomingXP(const FEffectProperties& Props)
{
	const float LocalIncomingXP = GetIncomingXP();
	SetIncomingXP(0.f);

	if (Props.SourceCharacter->Implements<UCombatInterface>() && Props.SourceCharacter->Implements<ULevelableInterface>())
	{
		// 레벨업 여부를 계산합니다.
		const int32 CurrentLevel = ICombatInterface::Execute_GetCharacterLevel(Props.SourceCharacter);
		const int32 CurrentXP = ILevelableInterface::Execute_GetXP(Props.SourceCharacter);

		const int32 NewLevel = ILevelableInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
		const int32 NumLevelUps = NewLevel - CurrentLevel;

		if (NumLevelUps > 0)
		{
			for (int32 i = 0; i < NumLevelUps; i++)
			{
				// 레벨이 상승한 경우, 레벨업 보상을 계산 및 부여합니다.
				const int32 AttributePointsReward = ILevelableInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentLevel + i);
				const int32 SpellPointsReward = ILevelableInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentLevel + i);
				ILevelableInterface::Execute_AddToAttributePoints(Props.SourceCharacter, AttributePointsReward);
				ILevelableInterface::Execute_AddToSpellPoints(Props.SourceCharacter, SpellPointsReward);
			}

			ILevelableInterface::Execute_AddToLevel(Props.SourceCharacter, NumLevelUps);

			// 레벨에 따른 MaxHealth와 MaxMana 반영 이후 Health, Mana를 최대치로 회복하기 위해 기록해둡니다. 
			bTopOffHealth = true;
			bTopOffMana = true;
			
			ILevelableInterface::Execute_LevelUp(Props.SourceCharacter);
		}

		// XP 보상을 부여합니다.
		ILevelableInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
}

void UAuraAttributeSet::ApplyDebuff(const FEffectProperties& Props) const
{
	const FGameplayEffectContextHandle EffectContextHandle = Props.EffectContextHandle;
	
	const FDebuffDataContext DebuffData = UAuraAbilitySystemLibrary::GetDebuffData(EffectContextHandle);
	
	// 이펙트 적용 시 GrantedTag가 자동으로 부여되므로, 그 전에 이미 해당 디버프가 부여되어있는지 확인해 나이아가라가 중복으로 생기지 않도록 방지합니다.
	const FGameplayTag DebuffTypeTag = UAuraAbilitySystemLibrary::ReplaceDebuffTypeToTag(DebuffData.DebuffType);
	if (!Props.TargetASC->HasMatchingGameplayTag(DebuffTypeTag))
	{
		if (Props.TargetCharacter)
		{
			if (UDebuffNiagaraComponent* NiagaraComponent = NewObject<UDebuffNiagaraComponent>(Props.TargetCharacter))
			{
				NiagaraComponent->DebuffTag = DebuffTypeTag;
				NiagaraComponent->SetupAttachment(Props.TargetCharacter->GetMesh(), FName("RootSocket"));
				NiagaraComponent->RegisterComponent();
			}
		}
	}

	switch (DebuffData.DebuffType)
	{
	case EDebuffTypeContext::Burn:
		ApplyBurnDebuff(Props, EffectContextHandle, DebuffData);
		break;
	case EDebuffTypeContext::Stun:
		ApplyStunDebuff(Props, EffectContextHandle, DebuffData);
		break;
	default:
		break;
	}
}

void UAuraAttributeSet::InitDebuffEffect(UGameplayEffect* DebuffEffect, const FDebuffDataContext& DebuffData) const
{
	DebuffEffect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DebuffEffect->Period = DebuffData.DebuffFrequency;
	DebuffEffect->bExecutePeriodicEffectOnApplication = false;
	DebuffEffect->DurationMagnitude = FScalableFloat(DebuffData.DebuffDuration);
     
	DebuffEffect->StackingType = EGameplayEffectStackingType::AggregateBySource;
	DebuffEffect->StackLimitCount = 1;
}

void UAuraAttributeSet::ApplyBurnDebuff(const FEffectProperties& Props, FGameplayEffectContextHandle EffectContextHandle, const FDebuffDataContext& DebuffData) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	const FGameplayTag DamageType = GameplayTags.Damage_Fire;
     
	// 동적으로 새로운 GE를 생성합니다.
	const FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *UAuraAbilitySystemLibrary::ReplaceDebuffTypeToTag(DebuffData.DebuffType).ToString());
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DebuffName));
	InitDebuffEffect(Effect, DebuffData);
	
	const int32 Index = Effect->Modifiers.Num();
	Effect->Modifiers.Add(FGameplayModifierInfo());
	FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];
	
	ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffData.DebuffDamage);
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.Attribute = GetIncomingDamageAttribute();
     
	// Context를 그대로 재사용해 EffectSpec을 생성합니다.
	// 애초에 Damage Effect가 아니였기 때문에 새로운 Damage 관련 정보를 Context에 할당해도 무방합니다.
	FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContextHandle, 1.f);
	if (MutableSpec)
	{
		MutableSpec->DynamicGrantedTags.AddTag(UAuraAbilitySystemLibrary::ReplaceDebuffTypeToTag(DebuffData.DebuffType));
		FDamageDataContext DamageData = UAuraAbilitySystemLibrary::GetDamageData(EffectContextHandle);
		DamageData.DamageType = UAuraAbilitySystemLibrary::ReplaceDamageTypeToEnum(DamageType);
		
		FAuraGameplayEffectContext* AuraContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
		AuraContext->SetDamageDataContext(DamageData.DamageType, DamageData.bIsBlockedHit, DamageData.bIsCriticalHit);
     
		Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);
	}
}

void UAuraAttributeSet::ApplyStunDebuff(const FEffectProperties& Props, FGameplayEffectContextHandle EffectContextHandle, const FDebuffDataContext& DebuffData) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	const FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *UAuraAbilitySystemLibrary::ReplaceDebuffTypeToTag(DebuffData.DebuffType).ToString());
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DebuffName));
	InitDebuffEffect(Effect, DebuffData);

	FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContextHandle, 1.f);
	if (MutableSpec)
	{
		MutableSpec->DynamicGrantedTags.AddTag(UAuraAbilitySystemLibrary::ReplaceDebuffTypeToTag(DebuffData.DebuffType));
		MutableSpec->DynamicGrantedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
		MutableSpec->DynamicGrantedTags.AddTag(GameplayTags.Player_Block_InputHeld);
		MutableSpec->DynamicGrantedTags.AddTag(GameplayTags.Player_Block_InputPressed);
		MutableSpec->DynamicGrantedTags.AddTag(GameplayTags.Player_Block_InputReleased);
		
		Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);

		FGameplayTagContainer Tags;
		Props.TargetASC->GetOwnedGameplayTags(Tags);
	}
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	// GAS 후처리 흐름을 자동으로 실행하는 매크로로서, 클라이언트 예측 등의 로직이 여기에 포함
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, FireResistance, OldFireResistance);
}

void UAuraAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, LightningResistance, OldLightningResistance);
}

void UAuraAttributeSet::OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArcaneResistance, OldArcaneResistance);
}

void UAuraAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, PhysicalResistance, OldPhysicalResistance);
}

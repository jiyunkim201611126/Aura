#include "AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "Abilities/AuraGameplayAbility.h"
#include "Aura/Interaction/LevelableInterface.h"
#include "AuraAbilitySystemLibrary.h"
#include "Aura/Game/SaveGame/AuraSaveGame.h"
#include "Data/AbilityInfo.h"
#include "Net/UnrealNetwork.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ThisClass::ClientEffectApplied);
}

void UAuraAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(UAuraAbilitySystemComponent, AdditionalCostManagers, COND_OwnerOnly);
}

void UAuraAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(UAuraSaveGame* SaveData)
{	
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	
	for (const FSavedAbility& Data : SaveData->SavedAbilities)
	{
		const FAuraAbilityInfo AbilityInfoStruct = AbilityInfo->FindAbilityInfoForTag(Data.AbilityTag);
		const TSubclassOf<UGameplayAbility> StartupAbilityClass = AbilityInfoStruct.Ability;
		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(StartupAbilityClass, Data.AbilityLevel);

		LoadedAbilitySpec.GetDynamicSpecSourceTags().AddTag(Data.InputTag);
		LoadedAbilitySpec.GetDynamicSpecSourceTags().AddTag(Data.AbilityStatus);

		const FAuraGameplayTags& AuraGameplayTags = FAuraGameplayTags::Get();

		const bool bIsActiveAbility = AbilityInfoStruct.AbilityType == AuraGameplayTags.Abilities_Types_Active;
		const bool bIsEquippedAbility = Data.AbilityStatus == AuraGameplayTags.Abilities_Status_Equipped;
		
		if (bIsActiveAbility)
		{
			// 액티브 Ability인 경우 들어오는 분기입니다.
			GiveAbility(LoadedAbilitySpec);
		}
		else
		{
			// 패시브 Ability인 경우 들어오는 분기입니다.
			// Equip된 Ability인 경우 장착과 동시에 발동하며, 그렇지 않은 경우 Ability 부여만 진행합니다.
			bIsEquippedAbility ? GiveAbilityAndActivateOnce(LoadedAbilitySpec) : GiveAbility(LoadedAbilitySpec);
		}
		
		if (Data.InputTag.IsValid())
		{
			ServerEquipAbility(Data.AbilityTag, Data.InputTag);
		}
	}
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : InAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{			
			// Ability를 미장착 상태로 변경합니다.
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Unlocked);
			
			// Ability를 부여합니다.
			// 부여란 Ability를 ASC에 등록하는 행위이며, 사용은 못 하는 상태일 수 있습니다.
			GiveAbility(AbilitySpec);

			// Ability를 장착합니다.
			// 장착이란 Ability를 사용 가능한 상태로 바꾸는 행위입니다.
			ServerEquipAbility(GetAbilityTagFromSpec(AbilitySpec), AuraAbility->StartupInputTag);
			
			// 부여된 Ability를 HUD에 스킬 아이콘으로 표시하기 위해 델리게이트를 Broadcast합니다.
			// 해당 함수는 서버에서만 호출되기 때문에, 클라이언트에서 OnRep 함수로 따로 Broadcast해줍니다.
			OnAbilityEquipped.Broadcast(GetAbilityTagFromSpec(AbilitySpec), AuraAbility->StartupInputTag, FAuraGameplayTags::Get().Abilities_Status_Equipped, FGameplayTag());
		}
	}
}

void UAuraAbilitySystemComponent::AddAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : InAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		GiveAbility(AbilitySpec);
			
		if (UAuraGameplayAbility* Ability = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			Ability->RegisterAbilityToAdditionalCostManagers(this);
		}
	}
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : InAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
		// PassiveAbility이므로, 부여와 동시에 발동합니다.
		GiveAbilityAndActivateOnce(AbilitySpec);
			
		if (UAuraGameplayAbility* Ability = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			Ability->RegisterAbilityToAdditionalCostManagers(this);
		}
	}
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	FScopedAbilityListLock ActiveScopeLock(*this);
	TMap<FGameplayAbilitySpecHandle, FGameplayAbilitySpec> NewAbilities;
	NewAbilities.Reserve(GetActivatableAbilities().Num());

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.Handle.IsValid())
		{
			continue;
		}

		NewAbilities.Add({AbilitySpec.Handle, AbilitySpec});

		// 이전에 갖고 있지 않던 Ability의 부여 상황을 UI에 알립니다.
		if (!CachedAbilities.Contains(AbilitySpec.Handle))
		{
			FGameplayTag InputTag;
			FGameplayTag StatusTag;
			ExtractTagsFromSpec(AbilitySpec, InputTag, StatusTag);
			OnAbilityEquipped.Broadcast(GetAbilityTagFromSpec(AbilitySpec), InputTag, StatusTag, FGameplayTag());
		}
		else
		{
			// 이미 캐싱된 Ability일 때 들어오는 분기입니다.
			const FGameplayAbilitySpec& CachedAbility = *CachedAbilities.Find(AbilitySpec.Handle);
			
			const FGameplayTag& AbilityTag = GetAbilityTagFromSpec(AbilitySpec);
			FGameplayTag PreviousInputTag;
			FGameplayTag PreviousStatusTag;
			FGameplayTag NowInputTag;
			FGameplayTag NowStatusTag;
			ExtractTagsFromSpec(CachedAbility, PreviousInputTag, PreviousStatusTag);
			ExtractTagsFromSpec(AbilitySpec, NowInputTag, NowStatusTag);
			
			const int32 PreviousLevel = CachedAbility.Level;
			const int32 NowLevel = AbilitySpec.Level;
			
			// InputTag의 변경 사항을 체크합니다.
			if (PreviousInputTag != NowInputTag && NowInputTag.IsValid())
			{
				// 현재 할당된 InputTag와 이전 InputTag가 다를 경우 UI에 알립니다.
				OnAbilityEquipped.Broadcast(AbilityTag, NowInputTag, NowStatusTag, PreviousInputTag);
			}

			// StatusTag와 Level의 변경 사항을 체크합니다.
			if (PreviousStatusTag != NowStatusTag || PreviousLevel != NowLevel)
			{
				OnAbilityStatusOrLevelChangedDelegate.Broadcast(AbilityTag, NowStatusTag, NowLevel);
			}
		}
		
		// 프로젝트 특성상 Ability 부여가 해제되는 상황은 존재하지 않습니다.
	}

	// 현재 Abilities 정보를 캐싱합니다.
	CachedAbilities = MoveTemp(NewAbilities);
}

void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}
	
	FScopedAbilityListLock ActiveScopeLock(*this);
	// 현재 장착 중인 Ability를 가져옵니다.
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 장착 중인 Ability 중 Tag가 일치하는 Ability가 있는지 탐색합니다.
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			// InputTag에 해당하는 Ability를 발동합니다.
			AbilitySpecInputPressed(AbilitySpec);

			if (AbilitySpec.IsActive())
			{
				// Ability가 WaitInput Task를 생성하면 해당 PredictionKey를 키로 하여 델리게이트에 콜백을 걸고, 이벤트를 발동하면 Task가 수신합니다.
				// XP와 Attribute 증가 로직 구현에 사용했던 SendGameplayEventToActor와 작동 원리가 일치합니다.
				// 5.5 이후로 ActivationInfo에 경고 문구가 출력되고 있으나, Lyra에서도 아래와 같이 구현하고 있는 걸 봐서 아직은 대체할 방식이 전부 구현되진 않은 것으로 보입니다.
PRAGMA_DISABLE_DEPRECATION_WARNINGS
				const UGameplayAbility* Instance = AbilitySpec.GetPrimaryInstance();
				FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : AbilitySpec.ActivationInfo.GetActivationPredictionKey();
PRAGMA_ENABLE_DEPRECATION_WARNINGS
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, OriginalPredictionKey);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock ActiveScopeLock(*this);
	// 현재 장착 중인 Ability를 가져옵니다.
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 장착 중인 Ability 중 Tag가 일치하는 Ability가 있는지 탐색합니다.
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			// Ability가 이미 작동 중이라면 더 작동시키지 않습니다.
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);

			if (AbilitySpec.IsActive())
			{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
				const UGameplayAbility* Instance = AbilitySpec.GetPrimaryInstance();
				FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : AbilitySpec.ActivationInfo.GetActivationPredictionKey();
PRAGMA_ENABLE_DEPRECATION_WARNINGS
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, OriginalPredictionKey);
			}
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	// 매개변수로 들어온 AbilitySpec의 Abilities 태그를 찾아 반환합니다.
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->GetAssetTags())
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities")))
				&& !Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status")))
				&& !Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Types"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	// 매개변수로 들어온 AbilitySpec의 InputTag를 찾아 반환합니다.
	for (FGameplayTag Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetGivenAbilitySpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

void UAuraAbilitySystemComponent::ExtractTagsFromSpec(const FGameplayAbilitySpec& AbilitySpec, FGameplayTag& OutInputTag, FGameplayTag& OutStatusTag) const
{
	// 여러 태그가 필요할 때 위 헬퍼 함수들을 사용하는 경우 불필요한 반복문이 호출됩니다.
	// 한 번의 반복 안에서 필요한 모든 태그를 가져오는 헬퍼함수입니다.

	const FGameplayTag& InputRootTag = FGameplayTag::RequestGameplayTag(FName("InputTag"));
	const FGameplayTag& StatusRootTag = FGameplayTag::RequestGameplayTag(FName("Abilities.Status"));

	for (const FGameplayTag& Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTag(InputRootTag))
		{
			OutInputTag = Tag;
		}
		else if (Tag.MatchesTag(StatusRootTag))
		{
			OutStatusTag = Tag;
		}

		if (OutInputTag.IsValid() && OutStatusTag.IsValid())
		{
			return;
		}
	}
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetGivenAbilitySpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	// 보유한 Ability 중 매개변수로 들어온 태그와 일치하는 태그를 가진 AbilitySpec을 반환합니다.
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->GetAssetTags())
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}

	// nullptr이 반환되면 보유하지 않은 Ability라는 걸 알 수 있습니다.
	return nullptr;
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FText& OutDescription, FText& OutNextLevelDescription, UAbilityInfo* Info)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetGivenAbilitySpecFromAbilityTag(AbilityTag))
	{
		// Ability가 부여되어있는 경우 들어오는 분기입니다.
		if (UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetDescription(AbilitySpec->Level + 1);
			return true;
		}
	}

	// Ability Tag가 None이 아니며, Ability가 부여되지 않은 상태라면 여기로 내려옵니다.
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None))
	{
		// Ability Tag가 유효하지 않거나 None인 경우, 즉 아직 개발되지 않은 Ability인 경우 들어오는 분기입니다.
		OutDescription = FText();
	}
	else
	{
		OutDescription = UAuraGameplayAbility::GetLockedDescription(Info->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	OutNextLevelDescription = FText();
	return false;
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<ULevelableInterface>())
	{
		if (ILevelableInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	if (GetAvatarActor()->Implements<ULevelableInterface>())
	{
		ILevelableInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	// 특정 레벨을 만족하면 Ability를 부여하는 함수입니다.
	// 장착과 부여는 다릅니다.
	// 부여란 GiveAbility에 의해 ASC에 등록되는 걸 말합니다.
	// 장착은 부여받은 Ability를 실제 사용 가능한 상태로 바꾸는 것을 말합니다.
	
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid())
		{
			continue;
		}
		
		if (Level < Info.LevelRequirement)
		{
			// 장착 가능 레벨 미만인 경우 못 한 경우 다음 Ability를 확인합니다.
			continue;
		}
		
		if (GetGivenAbilitySpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			// 장착 조건을 만족했으나 아직 부여되지 않은 경우 들어오는 분기입니다.
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);

			// Status 태그를 Eligible로 설정한 상태로 일단 GiveAbility를 통해 부여합니다.
			// 레벨을 만족하면 일단 '부여'하고, 추후 플레이어가 SpellPoint를 소모해 '장착'하면 Status 태그가 Equipped로 변경됩니다.
			// 추후 디버깅 시 장착하지 않은 Ability를 보고 '왜 부여됐지?'하며 헷갈릴 수 있으나, 능력 해금 시스템이 복잡해지거나 UI 상시 노출이 필요한 프로젝트의 경우 실용적인 로직입니다.
			// 특히 ASC에 접근해 GetActivatableAbilities로 '배운 Ability와 장착 가능한 Ability'를 빠르게 가져올 수 있다는 장점이 있습니다.
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);
			
			// 해당 변경 사항을 클라이언트에게 알려줍니다.
			MarkAbilitySpecDirty(AbilitySpec);
			
			// 리슨 서버도 UI에 반영할 수 있도록 델리게이트를 호출합니다.
			// 클라이언트는 OnRep_ActivateAbilities에서 변경 사항을 독자적으로 추적해 UI에 반영합니다.
			OnAbilityStatusOrLevelChangedDelegate.Broadcast(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	// 부여된 Ability 중 매개변수로 들어온 AbilityTag와 일치하는 태그를 가진 Ability를 탐색합니다.
	if (FGameplayAbilitySpec* AbilitySpec = GetGivenAbilitySpecFromAbilityTag(AbilityTag))
	{
		if (const UAbilityInfo* Info = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor()))
		{
			const FAuraAbilityInfo AbilityInfo = Info->FindAbilityInfoForTag(AbilityTag);
			if (AbilityInfo.MaxLevel <= AbilitySpec->Level)
			{
				// Ability의 현재 레벨이 최대 레벨 이상인 경우 SpellPoint 사용을 중단합니다.
				// 스패밍 방지를 위해 클라이언트도 현재 Ability 레벨을 참조해 SpendSpellPoint 버튼을 비활성화합니다.
				return;
			}
		}
		
		// 로직 흐름이 여기로 내려왔다면 SpellPoint를 반드시 소모해야 하는 상황입니다.
		if (GetAvatarActor()->Implements<ULevelableInterface>())
		{
			ILevelableInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}
		
		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		FGameplayTag StatusTag = GetStatusFromSpec(*AbilitySpec);

		if (StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			// Status가 Eligible이라면 Unlocked로 교체합니다.
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Unlocked);
			StatusTag = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			// Status가 이미 Equipped거나 Unlocked라면 레벨에 1 더합니다.
			AbilitySpec->Level++;
		}
		// 해당 변경 사항을 클라이언트에게도 알려줍니다.
		MarkAbilitySpecDirty(*AbilitySpec);
		// 리슨 서버도 UI에 반영할 수 있도록 델리게이트를 호출합니다.
		// 클라이언트는 OnRep_ActivateAbilities에서 변경 사항을 독자적으로 추적해 UI에 반영합니다.
		OnAbilityStatusOrLevelChangedDelegate.Broadcast(AbilityTag, StatusTag, AbilitySpec->Level);
	}
}

void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetGivenAbilitySpecFromAbilityTag(AbilityTag))
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		const FGameplayTag& PreviousInputTag = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& StatusTag = GetStatusFromSpec(*AbilitySpec);

		// 장착 가능한 Ability인지 판별합니다.
		const bool bStatusValid = StatusTag == GameplayTags.Abilities_Status_Equipped || StatusTag == GameplayTags.Abilities_Status_Unlocked;
		if (bStatusValid)
		{
			// 우선 장착하려는 InputTag에 있는 Ability의 InputTag를 제거합니다. 즉, 장착을 해제합니다.
			ClearAbilityOfInputTag(AbilitySpec, InputTag);
			// 장착하려는 Ability 또한 InputTag를 제거합니다. 마찬가지로 장착을 해제한다는 뜻입니다.
			ClearInputTag(AbilitySpec);
			
			// Ability를 InputTag에 장착합니다.
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(InputTag);

			// Ability가 부여 상태였다면 장착 상태로 변경합니다.
			if (StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
			{
				AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Unlocked);
				AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Equipped);
			}

			// Ability가 장착되었으므로, AdditionalCostManagers에게 이를 알려줍니다.
			if (UAuraGameplayAbility* Ability = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
			{
				Ability->RegisterAbilityToAdditionalCostManagers(this);
			}

			if (IsPassiveAbility(*AbilitySpec) && !AbilitySpec->IsActive())
			{
				TryActivateAbility(AbilitySpec->Handle);
			}

			// 클라이언트에게 Ability의 변경사항을 알려줍니다.
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		
		// 리슨 서버가 UI에 변경 사항을 표시할 수 있도록 델리게이트를 호출합니다.
		// 클라이언트는 OnRep_ActivateAbilities 함수에서 각종 Tag를 비교 및 변경 사항을 독자적으로 추적해 UI에 반영합니다.
		OnAbilityEquipped.Broadcast(AbilityTag, InputTag, GameplayTags.Abilities_Status_Equipped, PreviousInputTag);
	}
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	// Widget Controller에게 Tag를 가진 Effect가 Apply되었음을 알림
	EffectAssetTags.Broadcast(TagContainer);
}

void UAuraAbilitySystemComponent::ClearInputTag(FGameplayAbilitySpec* Spec)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	// 이 함수로 들어오는 관련 Ability는 장착 위치를 변경하는 중일 수 있습니다.
	// 따라서 Status 태그 변경은 하지 않으며, AdditionalCostManager에게도 접근하지 않습니다.
	const FGameplayTag InputTag = GetInputTagFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(InputTag);
}

void UAuraAbilitySystemComponent::ClearAbilityOfInputTag(const FGameplayAbilitySpec* AbilitySpecToEquip, const FGameplayTag& InputTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	// 현재 부여된 모든 Ability를 순회합니다.
	for (FGameplayAbilitySpec& GivenAbilitySpec : GetActivatableAbilities())
	{
		// '장착하려는 Ability'와 '장착하려는 InputTag에 이미 장착된 Ability'가 서로 다른 경우, 즉 이미 장착된 Ability를 해제할 때 들어가는 분기입니다.
		if (AbilityHasInputTag(GivenAbilitySpec, InputTag) && &GivenAbilitySpec != AbilitySpecToEquip)
		{
			if (UAuraGameplayAbility* GivenAbility = Cast<UAuraGameplayAbility>(GivenAbilitySpec.Ability))
			{
				// 장착 해제하는 중이므로, Manager 객체들에게서 이 Ability 관련 로직을 중단시킵니다.
				GivenAbility->UnregisterAbilityFromAdditionalCostManagers(this);
			}

			// 이미 장착되어있던 다른 Ability가 해제되는 상황인 게 확실하므로, StatusTag를 변경합니다.
			const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
			GivenAbilitySpec.GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Equipped);
			GivenAbilitySpec.GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Unlocked);

			if (IsPassiveAbility(GivenAbilitySpec))
			{
				// 해제하는 Ability가 Passive Ability인 경우 이를 알립니다.
				OnDeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(GivenAbilitySpec));
			}

			// Ability의 InputTag를 제거합니다.
			ClearInputTag(&GivenAbilitySpec);

			// 클라이언트에게 Ability의 변경사항을 알려줍니다.
			MarkAbilitySpecDirty(GivenAbilitySpec);
			
			return;
		}
	}
}

bool UAuraAbilitySystemComponent::AbilityHasInputTag(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& InputTag) const
{
	return AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag);
}

bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& AbilitySpec) const
{
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(AbilitySpec);
	const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;
	return AbilityType.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Types_Passive);
}

#include "AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "Abilities/AuraGameplayAbility.h"
#include "Aura/Interaction/LevelableInterface.h"
#include "AuraAbilitySystemLibrary.h"
#include "Data/AbilityInfo.h"
#include "Net/UnrealNetwork.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ThisClass::ClientEffectApplied);
}

void UAuraAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(UAuraAbilitySystemComponent, AbilityManagers, COND_OwnerOnly);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			// 어떤 Input에 의해 작동할 Ability인지 DynamicTag로 추가합니다.
			// DynamicTag는 AssetTag와 달리 런타임 중 자유롭게 Tag를 추가 및 제거할 수 있습니다.
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->StartupInputTag);
			
			// Ability를 장착 상태로 변경합니다.
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			
			// Ability를 부여합니다.
			// 부여란 Ability를 ASC에 등록하는 행위이며, 사용은 못 하는 상태일 수 있습니다.
			// 장착이란 Ability를 사용 가능한 상태로 바꾸는 행위입니다.
			GiveAbility(AbilitySpec);
			
			// 부여된 Ability를 HUD에 스킬 아이콘으로 표시하기 위해 델리게이트를 Broadcast합니다.
			// 해당 함수는 서버에서만 호출되기 때문에, 클라이언트에서 OnRep 함수로 따로 Broadcast해줍니다.
			OnAbilityEquipped.Broadcast(GetAbilityTagFromSpec(AbilitySpec), AuraAbility->StartupInputTag, FAuraGameplayTags::Get().Abilities_Status_Equipped, FGameplayTag());
		}
	}
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		// PassiveAbility이므로, 부여와 동시에 발동합니다.
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::AddAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		GiveAbility(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

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
			if (PreviousInputTag != NowInputTag)
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

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// 현재 장착 중인 Ability 가져오기
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 장착 중인 Ability 중 Tag가 일치하는 Ability가 있는지 탐색
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			// Pressed 이벤트도 발생시킴
			AbilitySpecInputPressed(AbilitySpec);
			// Ability가 이미 작동 중이라면 더 작동시키지 않음
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
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
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
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

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FText& OutDescription, FText& OutNextLevelDescription, UAbilityInfo* AbilityInfo)
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
		OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
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
			ClearAbilitiesOfInputTag(InputTag);
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

			// 클라이언트에게 Ability의 변경사항을 알려줍니다.
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		
		// 리슨 서버가 UI에 변경 사항을 표시할 수 있도록 델리게이트를 호출합니다.
		// 클라이언트는 OnRep_ActivateAbilities 함수에서 각종 Tag를 비교 및 변경 사항을 독자적으로 추적해 UI에 반영합니다.
		OnAbilityEquipped.Broadcast(AbilityTag, InputTag, StatusTag, PreviousInputTag);
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
	const FGameplayTag InputTag = GetInputTagFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(InputTag);
	MarkAbilitySpecDirty(*Spec);
}

void UAuraAbilitySystemComponent::ClearAbilitiesOfInputTag(const FGameplayTag& InputTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasInputTag(&Spec, InputTag))
		{
			ClearInputTag(&Spec);
		}
	}
}

bool UAuraAbilitySystemComponent::AbilityHasInputTag(FGameplayAbilitySpec* Spec, const FGameplayTag& InputTag) const
{
	for (FGameplayTag Tag : Spec->GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTagExact(InputTag))
		{
			return true;
		}
	}
	return false;
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	// 부여된 Ability 중 매개변수로 들어온 AbilityTag와 일치하는 태그를 가진 Ability를 탐색합니다.
	if (FGameplayAbilitySpec* AbilitySpec = GetGivenAbilitySpecFromAbilityTag(AbilityTag))
	{
		// 이 분기로 들어왔다면 SpellPoint를 반드시 소모해야 하는 상황입니다.
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

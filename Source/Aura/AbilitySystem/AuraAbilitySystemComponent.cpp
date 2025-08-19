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
			// 어떤 Input에 의해 사용될 Ability인지 기록
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->StartupInputTag);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			// Ability 장착
			GiveAbility(AbilitySpec);
			// 부여된 Ability를 HUD에 스킬 아이콘으로 표시하기 위해 델리게이트를 Broadcast합니다.
			// 해당 함수는 서버에서만 호출되기 때문에, 클라이언트에서 OnRep 함수로 따로 Broadcast해줍니다.
			AbilitiesGivenDelegate.Broadcast(AbilitySpec);
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

	TSet<FGameplayAbilitySpecHandle> NewHandles;
	NewHandles.Reserve(GetActivatableAbilities().Num());

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.Handle.IsValid())
		{
			continue;
		}

		NewHandles.Add(AbilitySpec.Handle);

		// 이전에 갖고 있지 않던 Ability만 위젯에 알려줍니다.
		if (!CachedAbilityHandles.Contains(AbilitySpec.Handle))
		{
			AbilitiesGivenDelegate.Broadcast(AbilitySpec);
		}
	}

	// 추후 Ability가 제거되는 로직이 추가되면 여기에 로직을 작성해 위젯이 알 수 있도록 해줍니다.

	CachedAbilityHandles = MoveTemp(NewHandles);
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

void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	// 특정 레벨을 만족하면 Ability를 부여하는 함수입니다.
	// 습득과 부여는 다릅니다.
	// 부여란 GiveAbility에 의해 ASC에 등록되는 걸 말합니다.
	// 습득은 부여받은 Ability를 실제 사용 가능한 상태로 바꾸는 것을 말합니다.
	
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid())
		{
			continue;
		}
		
		if (Level < Info.LevelRequirement)
		{
			// 습득 가능 레벨 미만인 경우 못 한 경우 다음 Ability를 확인합니다.
			continue;
		}
		
		if (GetGivenAbilitySpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			// 습득 조건을 만족했으나 아직 부여되지 않은 경우 들어오는 분기입니다.
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);

			// Status 태그를 Eligible로 설정한 상태로 일단 GiveAbility를 통해 부여합니다.
			// 레벨을 만족하면 일단 '부여'하고, 추후 플레이어가 SpellPoint를 소모해 '습득', Status 태그가 Equipped로 변경됩니다.
			// 추후 디버깅 시 습득하지 않은 Ability를 보고 '왜 부여됐지?'하며 헷갈릴 수 있으나, 능력 해금 시스템이 복잡해지거나 UI 상시 노출이 필요한 시스템의 경우 실용적인 로직입니다.
			// 특히 ASC에 접근해 GetActivatableAbilities로 '배운 Ability와 습득 가능한 Ability'를 빠르게 가져올 수 있다는 장점이 있습니다.
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec);
			ClientUpdateAbilityStatus(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FText& OutDescription, FText& OutNextLevelDescription)
{
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None))
	{
		// Ability Tag가 유효하지 않거나 None인 경우, 즉 아직 개발되지 않은 Ability인 경우 들어오는 분기입니다.
		OutDescription = FText();
		OutNextLevelDescription = FText();
		return false;
	}
	
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
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	OutNextLevelDescription = FText();
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
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);

		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			// Status가 Eligible이라면 Unlocked로 교체합니다.
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			// Status가 이미 Equipped거나 Unlocked라면 레벨에 1 더합니다.
			AbilitySpec->Level++;
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const int32 AbilityLevel)
{
	AbilityStatusChangedDelegate.Broadcast(AbilityTag, StatusTag, AbilityLevel);
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

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	// Widget Controller에게 Tag를 가진 Effect가 Apply되었음을 알림
	EffectAssetTags.Broadcast(TagContainer);
}

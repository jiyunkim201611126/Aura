#include "AuraGameplayAbility.h"

#include "Aura/Character/Component/StackableAbilityComponent.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Interaction/EnemyInterface.h"

void UAuraGameplayAbility::UpdateFacingToCombatTarget() const
{
	UObject* SourceActor = GetAvatarActorFromActorInfo();
	const AActor* TargetActor = IEnemyInterface::Execute_GetCombatTarget(SourceActor);
	const FVector TargetLocation = TargetActor->GetActorLocation();
	ICombatInterface::Execute_UpdateFacingTarget(SourceActor, TargetLocation);
}

FTaggedMontage UAuraGameplayAbility::GetRandomMontage()
{
	const int RandomIndex = FMath::RandRange(0, TaggedMontages.Num() - 1);
	FTaggedMontage TaggedMontage = TaggedMontages[RandomIndex];

	return TaggedMontage;
}

//*

void UAuraGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 이 Ability가 부여될 때, 대상에게 StackableAbilityComponent를 부여합니다.
	if (ActorInfo->IsNetAuthority())
	{
		if (UStackableAbilityComponent* Comp = GetStackableAbilityComponent(ActorInfo))
		{
			// 이 Ability의 충전 타이머를 등록합니다.
			Comp->RegisterAbility(AbilityTags.First(), StackData.MaxStack, StackData.RechargeTime);
		}
	}
}

bool UAuraGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 충전된 스택이 없다면 false를 반환합니다.
	if (UStackableAbilityComponent* Comp = GetStackableAbilityComponent(ActorInfo))
	{
		if (!Comp->CheckCost(AbilityTags.First()))
		{
			return false;
		}
	}
	
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UAuraGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// 충전된 스택을 소모합니다.
	if (UStackableAbilityComponent* Comp = GetStackableAbilityComponent(ActorInfo))
	{
		Comp->ApplyCost(AbilityTags.First());
	}
	
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void UAuraGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// 이 Ability가 제거될 때, 이 Ability의 충전 타이머를 제거합니다.
	if (UStackableAbilityComponent* Comp = GetStackableAbilityComponent(ActorInfo))
	{
		Comp->UnregisterAbility(AbilityTags.First());
	}
	
	Super::OnRemoveAbility(ActorInfo, Spec);
}

UStackableAbilityComponent* UAuraGameplayAbility::GetStackableAbilityComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	// 이미 StackableAbilityComponent가 있다면 그 컴포넌트에 이 Ability를 등록하고, 없다면 직접 스폰 후 붙여줍니다.
	if (AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr)
	{
		UStackableAbilityComponent* Comp = AvatarActor->FindComponentByClass<UStackableAbilityComponent>();
		if (Comp)
		{
			return Comp;
		}
		
		Comp = Cast<UStackableAbilityComponent>(AvatarActor->AddComponentByClass(UStackableAbilityComponent::StaticClass(), false, FTransform::Identity, true));
		AvatarActor->FinishAddComponent(Comp, false, FTransform::Identity);
		return Comp;
	}

	return nullptr;
}

//*/
#include "AuraGameplayAbility.h"

#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Interaction/EnemyInterface.h"

UAuraGameplayAbility::UAuraGameplayAbility()
{
	if (bUseCharges)
	{
		InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	}
}

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

void UAuraGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	StartRecharge();
}

bool UAuraGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (bUseCharges && CurrentCharges <= 0)
	{
		return false;
	}
	
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UAuraGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (bUseCharges)
	{
		// const_cast는 웬만해선 사용하지 말라고 들었지만, 충전 횟수 감소는 무조건 ApplyCost에서 하는 게 맞다고 생각이 들어 여기서 처리합니다.
		// ActivateAbility를 오버라이드해 CurrentCharges를 조작하면 Cost 소모가 제대로 이루어지지 않는 현상도 있습니다.
		auto* MutableThis = const_cast<UAuraGameplayAbility*>(this);
		MutableThis->CurrentCharges = FMath::Max(0, MutableThis->CurrentCharges - 1);
		MutableThis->StartRecharge();
	}
	
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void UAuraGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (bUseCharges)
	{
		StopRecharge();
	}
	
	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UAuraGameplayAbility::StartRecharge()
{
	UE_LOG(LogTemp, Log, TEXT("StartRecharge() -> CurrentCharges : %d"), CurrentCharges);
	if (!bUseCharges || CurrentCharges >= MaxCharges)
	{
		StopRecharge();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		// EndAbility 호출 이후 인스턴스의 멤버 함수 바인딩이 더이상 유효하지 않게 됩니다.
		// 따라서 델리게이트를 지역변수로 선언해 멤버변수로 넣어줍니다.
		FTimerDelegate Delegate;
		Delegate.BindLambda([this]()
		{
			this->Recharge();
		});
		
		World->GetTimerManager().SetTimer(
			RechargeTimerHandle,
			Delegate,
			RechargeTime,
			true);
	}
}

void UAuraGameplayAbility::StopRecharge()
{
	UE_LOG(LogTemp, Log, TEXT("StopRecharge() -> CurrentCharges : %d"), CurrentCharges);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RechargeTimerHandle);
	}
}

void UAuraGameplayAbility::Recharge()
{
	++CurrentCharges;

	if (CurrentCharges >= MaxCharges)
	{
		StopRecharge();
	}
	UE_LOG(LogTemp, Log, TEXT("Recharge() -> CurrentCharges : %d"), CurrentCharges);
}

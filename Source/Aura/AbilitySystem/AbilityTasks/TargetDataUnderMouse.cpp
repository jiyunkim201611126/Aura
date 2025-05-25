#include "TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"
#include "Aura/Player/AuraPlayerController.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	Super::Activate();

	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		SendMouseCursorData();
	}
	else
	{
		// 서버쪽 로직
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// GAS에서 네트워크 예측 작업의 범위를 명확히 지정해주는 역할
	// 해당 작업이 예측적으로 진행되고 있음을 GAS에 알리기 위해 사용
	// 여기서 PredictionKey란, 이 함수 흐름 전체에 Key를 부여해 다른 네트워크 작업과 섞이거나 충돌하지 않도록 방지함
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	AAuraPlayerController* PC = Cast<AAuraPlayerController>(Ability->GetCurrentActorInfo()->PlayerController.Get());
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = PC->CursorHit;
	DataHandle.Add(Data);
	
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),											// 서버가 어떤 Ability 인스턴스에 대한 TargetData인지 식별할 때 사용
		GetActivationPredictionKey(),									// Ability 실행 시점의 예측키. 동기화, 예측, 롤백 처리를 위해 사용
		DataHandle,														// TargetData에 대한 실제 정보를 담고 있는 핸들
		FGameplayTag(),													// 추가적인 태그 정보 전달을 위해 사용
		AbilitySystemComponent->ScopedPredictionKey);	// 이 작업이 예측적으로 발생했음을 설명하는 부가 정보

	// Ability를 실행해도 괜찮은지를 판별하는 함수
	// 네트워크 예측 작업으로 인해 델리게이트가 중복으로 호출되는 것을 방지함
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

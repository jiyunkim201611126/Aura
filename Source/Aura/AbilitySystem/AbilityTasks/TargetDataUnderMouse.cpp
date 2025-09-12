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
		// 클라이언트쪽 로직입니다.
		SendMouseCursorData();
	}
	else
	{
		// 서버쪽 로직입니다.

		// 현재 인스턴스를 식별하기 위한 고유 핸들
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		// 이 Ability가 실행될 때 생성된 예측키
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		// 서버가 TargetData를 수신했을 때 호출할 델리게이트 등록
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		// 이미 TargetData가 도착해있다면 위에서 등록한 콜백을 즉시 호출
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// 이 함수 흐름 전체에 예측 Key를 부여해 다른 네트워크 작업과 섞이거나 충돌하지 않도록 방지합니다.
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

	// PlayerController에서 HitResult 가져옵니다.
	AAuraPlayerController* PC = Cast<AAuraPlayerController>(Ability->GetCurrentActorInfo()->PlayerController.Get());
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = PC->CursorHit;
	DataHandle.Add(Data);
	
	/**
	 * Ability를 실행하게 되면 자동으로 예측키가 생성, 서버와의 소통을 시작합니다.(여기는 Task, 여기서 만든 거 아님)
	 * 아래 함수를 호출할 때 이 예측키를 2번째 매개변수로 사용해 '어떤 클라이언트에서, 어떤 Ability에서' 실행 중인지 GAS가 파악합니다.
	 * 마지막 매개변수로 들어간 예측키(여기서 만든 거 맞음)는 이 함수(SendMouseCursorData)의 첫 구문에서 선언되어 '이 함수'에서 예측 작업이 실행 중임을 명시합니다.
	 * 즉, GAS는 이 2개의 매개변수인 예측키를 통해 다른 클라이언트나 다른 Ability 혹은 다른 함수에서 실행 중인 예측 작업과 섞이지 않도록 제어합니다.
	 */
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),											// 서버가 어떤 Ability 인스턴스에 대한 TargetData인지 식별할 때 사용
		GetActivationPredictionKey(),									// Ability 실행 전체의 예측키. 동기화, 예측, 롤백 처리를 위해 사용
		DataHandle,														// TargetData에 대한 실제 정보를 담고 있는 핸들
		FGameplayTag(),													// 추가적인 태그 정보 전달을 위해 사용
		AbilitySystemComponent->ScopedPredictionKey);	// 이 함수의 예측키. 가장 윗줄에서 선언되었음

	// Task의 델리게이트를 호출해도 괜찮은지를 판별하는 함수입니다.
	// 네트워크 예측 작업으로 인해 델리게이트가 중복으로 호출되는 것을 방지합니다.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// 최종적으로 블루프린트에서 정의한 흐름을 호출, TargetData에 대한 정보를 담고 있는 핸들을 전달합니다.
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	// 이 Ability와 예측키에 해당하는 모든 네트워크로 복제된 데이터를 소모합니다.(중복 처리 방지)
	AbilitySystemComponent->ConsumeAllReplicatedData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

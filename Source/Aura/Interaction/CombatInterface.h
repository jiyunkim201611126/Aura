#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

/**
 * 교전과 관련된 각종 기능을 가진 인터페이스입니다.
 * 대부분의 함수가 BlueprintNativeEvent로 선언되어 있으나, 블루프린트 구현은 없는 경우가 다수 존재합니다.
 * 이는 나중의 확장성을 위해 BlueprintNativeEvent로 미리 선언해놓은 상태입니다.
 */

class UAnimMontage;

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

class AURA_API ICombatInterface
{
	GENERATED_BODY()

public:
	// PawnManagerSubsystem에 Pawn을 등록 및 해제하는 함수
	// Controller만 해도 작동엔 문제가 없으나, 멀티플레이 환경에서 강제종료 등의 상황 발생 시 문제가 생길 수 있으므로 Controller와 Character가 모두 한 번씩 호출
	virtual void RegisterPawn();
	virtual void UnregisterPawn();
	
	virtual int32 GetPlayerLevel();
	
	// 무기의 Socket으로부터 위치를 가져오는 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocation(const FGameplayTag& MontageTag, const FName& SocketName);
	
	// Motion Warping Component를 통해 캐릭터를 원하는 방향으로 회전시키는 기능을 블루프린트에서 구현 및 호출
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& FacingTarget);

	virtual void Die(bool bShouldAddImpulse, const FVector& Impulse) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDead();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* GetAvatar();
};

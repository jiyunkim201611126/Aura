#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SavedActorInterface.generated.h"

UINTERFACE()
class USavedActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 저장될 액터가 상속받을 인터페이스입니다.
 * 저장 로직 담당이 아닌 저장될 데이터를 가진 클래스가 해당 인터페이스를 상속받습니다.
 * 예) 맵 내 일회용 액터, Checkpoint 등
 */
class AURA_API ISavedActorInterface
{
	GENERATED_BODY()

public:
	// 아래 함수들을 BeginPlay와 Destroyed에서 반드시 호출합니다.
	void RegisterSavedActor(const UObject* WorldContextObject);
	void UnregisterSavedActor(const UObject* WorldContextObject);
};

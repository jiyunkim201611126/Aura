#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveGameInterface.generated.h"

UINTERFACE()
class USaveGameInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 게임 저장을 담당하는 클래스가 상속받을 인터페이스입니다.
 * 저장될 데이터를 가진 객체가 아닌, 저장 로직을 담당합니다.
 * 현재는 AuraCharacter만 상속받고 있으며, SaveProgress 함수를 Checkpoint 클래스가 호출합니다.
 */
class AURA_API ISaveGameInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveProgress(const FName& CheckpointTag);
};

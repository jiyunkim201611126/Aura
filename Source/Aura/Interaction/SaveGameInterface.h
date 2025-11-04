#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveGameInterface.generated.h"

UINTERFACE()
class USaveGameInterface : public UInterface
{
	GENERATED_BODY()
};

class AURA_API ISaveGameInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveProgress(const FName& CheckpointTag);
};

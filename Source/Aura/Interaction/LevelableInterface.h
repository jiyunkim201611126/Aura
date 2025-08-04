#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LevelableInterface.generated.h"

UINTERFACE()
class ULevelableInterface : public UInterface
{
	GENERATED_BODY()
};

class AURA_API ILevelableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	int32 FindLevelForXP(int32 InXP) const;
	
	UFUNCTION(BlueprintNativeEvent)
	int32 GetXP() const;

	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePointsReward(int32 Level) const;

	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPointsReward(int32 Level) const;
	
	UFUNCTION(BlueprintNativeEvent)
	void AddToXP(const int32 InXP);
	
	UFUNCTION(BlueprintNativeEvent)
	void AddToLevel(const int32 InPlayerLevel);
	
	UFUNCTION(BlueprintNativeEvent)
	void AddToAttributePoints(const int32 InAttributePoints);
	
	UFUNCTION(BlueprintNativeEvent)
	void AddToSpellPoints(const int32 InSpellPoints);

	UFUNCTION(BlueprintNativeEvent)
	void LevelUp();
};

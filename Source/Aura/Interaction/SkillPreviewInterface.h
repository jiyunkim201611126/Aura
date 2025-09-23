#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SkillPreviewInterface.generated.h"

UINTERFACE()
class USkillPreviewInterface : public UInterface
{
	GENERATED_BODY()
};

class AURA_API ISkillPreviewInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowSkillPreview(UMaterialInterface* DecalMaterial = nullptr);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideSkillPreview();
};

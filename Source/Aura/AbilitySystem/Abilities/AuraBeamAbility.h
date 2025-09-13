#pragma once

#include "CoreMinimal.h"
#include "AuraChannelingAbility.h"
#include "AuraBeamAbility.generated.h"

UCLASS()
class AURA_API UAuraBeamAbility : public UAuraChannelingAbility
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	void TraceFirstTarget();

	UFUNCTION(BlueprintCallable)
	void StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Beam")
	TObjectPtr<AActor> TargetHitActor;

	// Beam이 주변 적에게 전이되는 범위입니다.
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	float SplashRadius = 500.f;

	// 최대 추가 타격 가능한 적 수입니다.
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	int32 MaxHitTargets = 3;
};

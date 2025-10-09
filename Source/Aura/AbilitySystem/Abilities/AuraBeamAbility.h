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
	bool CheckRange();

	UFUNCTION(BlueprintCallable)
	void StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ApplyEndAbility();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Beam")
	TObjectPtr<AActor> TargetHitActor;

	// 사정거리입니다.
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	float BeamRange = 800.f;

	// Beam이 주변 적에게 전이되는 범위입니다.
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	float SplashRadius = 500.f;

	// 최대 타격 가능한 적 수입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Beam")
	int32 MaxHitTargets = 3;
};

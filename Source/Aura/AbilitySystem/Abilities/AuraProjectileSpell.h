#pragma once

#include "CoreMinimal.h"
#include "AuraDamageGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectile;

UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

public:
	virtual FText GetDescription_Implementation(int32 Level) override;

	UFUNCTION(BlueprintPure)
	int32 GetProjectileNumsToSpawn(int32 Level) const;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * Projectile을 생성 및 발사하는 함수입니다.
	 *
	 * @param ProjectileSpawnLocation 
	 * @param ProjectileTargetLocation 
	 * @param PitchOverride 양수 입력 시 해당 각도만큼 위쪽으로 발사됩니다.
	 * @param bHoming 투사체가 타겟을 추적할지 결정합니다. false로 호출 시 궤도 변경 없이 발사된 방향으로 나아갑니다.
	 * @param HomingTarget 추적할 타겟으로, bHoming이 true이며 nullptr인 경우 클릭한 위치로 나아갑니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(UPARAM(ref) FVector& ProjectileSpawnLocation, UPARAM(ref) FVector& ProjectileTargetLocation, const bool bHoming, const float PitchOverride, const AActor* HomingTarget);

	void SetHandlesToProjectile(AAuraProjectile* Projectile, const FVector& TargetLocation);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AAuraProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	FScalableFloat NumOfProjectiles = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ProjectileSpread = 30.f;

	// 추적하며 궤도를 변경하는 최소 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float HomingAccelerationMin = 1000.f;

	// 추적하며 궤도를 변경하는 최대 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float HomingAccelerationMax = 1000.f;
};

#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectile;

UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	int32 GetProjectileNumsToSpawn(int32 Level) const;

protected:
	UFUNCTION(BlueprintCallable)
	void SetTarget(const FGameplayAbilityTargetDataHandle& Handle);
	
	/**
	 * Projectile을 생성 및 발사하는 함수입니다.
	 *
	 * @param InProjectileSpawnLocation
	 * @param InProjectileTargetLocation 이미 멤버변수 ProjectileTargetLocation이 있지만, AI의 해당 Ability 사용을 위해 매개변수로도 선언합니다.
	 * @param PitchOverride 양수 입력 시 해당 각도만큼 위쪽으로 발사됩니다.
	*/
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	TArray<AAuraProjectile*> SpawnProjectile(UPARAM(ref) FVector& InProjectileSpawnLocation, UPARAM(ref) FVector& InProjectileTargetLocation, const float PitchOverride = 0.f);

	// EvenlySpacedRotators 대신 EvenlySpacedRotatorsWithCircle을 사용하는 함수입니다.
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	TArray<AAuraProjectile*> SpawnProjectileWithCircle(UPARAM(ref) FVector& InProjectileSpawnLocation, UPARAM(ref) FVector& InProjectileTargetLocation, const float PitchOverride = 0.f);

	TArray<AAuraProjectile*> SpawnProjectile(int32 NumProjectilesToSpawn, const FVector& InProjectileSpawnLocation, const FVector& InProjectileTargetLocation, const TArray<FRotator>& Rotations);

	void SetHandlesToProjectile(AAuraProjectile* Projectile, const FVector& TargetLocation) const;

protected:
	UPROPERTY(BlueprintReadWrite)
	FVector ProjectileTargetLocation;
	
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> HomingTarget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AAuraProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	FScalableFloat NumOfProjectiles = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileSpread = 30.f;

	// 타겟을 추적할지 결정하는 변수입니다.
	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool bHoming = false;

	// 추적하며 궤도를 변경하는 최소 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (EditCondition = "bHoming"))
	float HomingAccelerationMin = 1000.f;

	// 추적하며 궤도를 변경하는 최대 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (EditCondition = "bHoming"))
	float HomingAccelerationMax = 1000.f;
};

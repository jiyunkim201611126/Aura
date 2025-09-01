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

	// Montage의 Notify에 의해 실행되는 함수
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(UPARAM(ref) FVector& ProjectileSpawnLocation, UPARAM(ref) FVector& ProjectileTargetLocation);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AAuraProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	FScalableFloat NumOfProjectiles = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float ProjectileSpread = 30.f;
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AuraProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class AURA_API AAuraProjectile : public AActor
{
	GENERATED_BODY()

public:
	AAuraProjectile();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnHomingTargetComponent(const FVector_NetQuantize& TargetLocation, const float HomingAcceleration);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetHomingTargetComponent(USceneComponent* HomingTargetComponent, const float HomingAcceleration);

protected:
	// ~AActor Interface
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void Tick(float DeltaSeconds) override;
	// ~End of AActor Interface

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void PlayHitFXs() const;
	
public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY()
	FGameplayEffectContextHandle DamageEffectContextHandle;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	TArray<FGameplayEffectSpecHandle> DamageEffectSpecHandle;

	float DeathImpulseMagnitude = 0.f;
	float KnockbackForceMagnitude = 0.f;

	UPROPERTY()
	FGameplayEffectContextHandle DebuffEffectContextHandle;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	TArray<FGameplayEffectSpecHandle> DebuffEffectSpecHandle;

private:
	UPROPERTY(EditAnywhere)
	float LifeSpan = 1.f;

	// 클라이언트가 투사체의 이펙트(사운드, 나이아가라)가 중복되거나 누락되지 않게 하기 위한 변수
	bool bHit = false;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag LoopingSoundTag;

	UPROPERTY(EditAnywhere)
	FGameplayTag ImpactEffectTag;

	UPROPERTY(EditAnywhere)
	FGameplayTag ImpactSoundTag;
};

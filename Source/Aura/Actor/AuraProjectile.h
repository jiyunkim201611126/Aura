#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AuraProjectile.generated.h"

class UAbilityEffectPolicy;
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
	//~ Begin Actor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End Actor Interface

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void PlayHitFXs() const;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY()
	TWeakObjectPtr<UGameplayAbility> OwningAbility;

	UPROPERTY()
	TArray<TObjectPtr<UAbilityEffectPolicy>> EffectPolicies;

	// 한 번에 여러 개의 Projectile을 스폰하는 Ability의 경우 굉장히 시끄러우므로, 스폰 시점에 VolumeMultiplier를 설정하는 것으로 해결합니다.
	float VolumeMultiplier = 1.f;

private:
	UPROPERTY(EditAnywhere)
	bool bDestroyWithOverlap = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
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

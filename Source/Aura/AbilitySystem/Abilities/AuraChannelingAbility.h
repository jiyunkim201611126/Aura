#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayAbility.h"
#include "AuraChannelingAbility.generated.h"

class AAuraCharacterBase;

UCLASS()
class AURA_API UAuraChannelingAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void StoreMouseDataInfo(const FHitResult& HitResult);

	UFUNCTION(BlueprintCallable)
	void PlayLoopAnimMontage();

	UFUNCTION(BlueprintCallable)
	void StopLoopAnimMontage();
	
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Channeling")
	FVector MouseHitLocation;

	UPROPERTY(BlueprintReadWrite, Category = "Channeling")
	TObjectPtr<AActor> MouseHitActor;

	UPROPERTY(BlueprintReadOnly, Category = "Channeling")
	TObjectPtr<AAuraCharacterBase> OwnerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "Channeling")
	TObjectPtr<UAnimMontage> LoopAnimMontage;
};

#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayAbility.h"
#include "AuraChannelingAbility.generated.h"

UCLASS()
class AURA_API UAuraChannelingAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void StoreMouseDataInfo(const FHitResult& HitResult);

	UFUNCTION(BlueprintCallable)
	void StoreOwnerVariables();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Channeling")
	FVector MouseHitLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Channeling")
	TObjectPtr<AActor> MouseHitActor;

	UPROPERTY(BlueprintReadOnly, Category = "Channeling")
	TObjectPtr<ACharacter> OwnerCharacter;
};

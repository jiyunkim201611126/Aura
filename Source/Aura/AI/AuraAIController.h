#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AuraAIController.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

UCLASS()
class AURA_API AAuraAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAuraAIController();

	
	//~ Begin Controller Interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~ End Controller Interface

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};

#include "AuraAIController.h"

#include "Aura/Interaction/CombatInterface.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

AAuraAIController::AAuraAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	check(Blackboard);
	check(BehaviorTreeComponent);
}

void AAuraAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HasAuthority())
	{
		Cast<ICombatInterface>(InPawn)->RegisterPawn();
	}
}

void AAuraAIController::OnUnPossess()
{
	if (HasAuthority())
	{
		Cast<ICombatInterface>(GetPawn())->UnregisterPawn();
	}
	
	Super::OnUnPossess();
}

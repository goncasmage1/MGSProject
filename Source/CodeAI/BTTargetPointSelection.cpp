// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "BTTargetPointSelection.h"
#include "BotTargetPoint.h"
#include "AnAIController.h"
#include "MyAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTTargetPointSelection::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	AAnAIController* AICon = Cast<AAnAIController>(OwnerComp.GetAIOwner());

	if (AICon) {

		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComp();
		TArray<ABotTargetPoint*> AvailableTargetPoints = AICon->GetAvailableTargetPoints();

		//Find a next point which is different from the current one
		AMyAICharacter* MyChar = Cast<AMyAICharacter>(AICon->GetCharacter());

		if (MyChar) {

			//Update the next location in the Blackboard so the bot can move to the next Blackboard value
			AICon->SetTargetPoint(AvailableTargetPoints[MyChar->GetTargetNum()]);

			//Increment the AI Character's TargetIndex
			MyChar->IncrementTargetNum();

			return EBTNodeResult::Succeeded;
		}
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Failed;
}


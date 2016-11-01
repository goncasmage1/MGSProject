// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "BTClearValues.h"
#include "AnAIController.h"
#include "MyAICharacter.h"

EBTNodeResult::Type UBTClearValues::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	AAnAIController* AICon = Cast<AAnAIController>(OwnerComp.GetAIOwner());

	if (AICon) {
		AICon->SetState(EAIState::AI_Patrolling);
		AICon->SetSlowSpeed(true);
		AMyAICharacter* AIChar = Cast<AMyAICharacter>(AICon->GetCharacter());
		if (AIChar) {
			AIChar->SetNoiseHeard(false);
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}

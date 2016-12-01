// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "ChangePlayerSpeed.h"
#include "AnAIController.h"
#include "CodeAICharacter.h"
#include "MyAICharacter.h"

EBTNodeResult::Type UChangePlayerSpeed::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	AAnAIController* AICon = Cast<AAnAIController>(OwnerComp.GetAIOwner());

	if (AICon) {
		AMyAICharacter * MyAI = Cast<AMyAICharacter>(AICon->GetCharacter());
		if (MyAI) {
			if (!AICon->GetSlowSpeed()) {
				MyAI->GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
				MyAI->SetRunning(true);
			}
			else {
				MyAI->GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
				MyAI->SetRunning(false);
			}
			return EBTNodeResult::Succeeded;
		}
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Failed;
}

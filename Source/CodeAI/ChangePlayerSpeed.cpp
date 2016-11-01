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
		if (!AICon->GetSlowSpeed()) {
			AMyAICharacter * MyAI = Cast<AMyAICharacter>(AICon->GetCharacter());
			MyAI->GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
		}
		else {
			AMyAICharacter * MyAI = Cast<AMyAICharacter>(AICon->GetCharacter());
			MyAI->GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}

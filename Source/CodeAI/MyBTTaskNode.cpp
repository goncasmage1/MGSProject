// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "MyBTTaskNode.h"
#include "AnAIController.h"
#include "MyAICharacter.h"
#include "Kismet/KismetMathLibrary.h"

EBTNodeResult::Type UMyBTTaskNode::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	AAnAIController* AICon = Cast<AAnAIController>(OwnerComp.GetAIOwner());

	if (AICon) {
		FVector Loc = AICon->GetHeardLocation();
		FVector CharLoc = AICon->GetCharacter()->GetActorLocation();
		Loc.Z = CharLoc.Z;

		FRotator Rot = UKismetMathLibrary::FindLookAtRotation(CharLoc, Loc);
		AICon->GetCharacter()->SetActorRotation(Rot);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

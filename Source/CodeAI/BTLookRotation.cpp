// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "BTLookRotation.h"
#include "AnAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AI/Navigation/NavigationSystem.h"

EBTNodeResult::Type UBTLookRotation::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	AAnAIController* AICon = Cast<AAnAIController>(OwnerComp.GetAIOwner());

	if (AICon) {

		//Clears the OriginalLocation value
		AICon->ClearNewLocation();

		//50% chance of executing each action
		float Random = FMath::RandRange(0, 1);

		//Finds a random rotation and faces that way
		if (Random) {
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Looking"));
			PawnRotation = AICon->GetPawn()->GetActorRotation();
			PawnRotation.Yaw = UKismetMathLibrary::RandomRotator().Yaw;
			AICon->GetPawn()->SetActorRotation(PawnRotation.Quaternion());
			return EBTNodeResult::Succeeded;
		}
		//Finds a random point in the NavMesh and moves to that point
		else {
			UNavigationSystem* NavSys = UNavigationSystem::GetCurrent(GetWorld());
			FNavLocation NavLoc;
			
			if (NavSys->GetRandomPointInNavigableRadius(AICon->GetPawn()->GetActorLocation(), 4000.f, NavLoc)) {

				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Found new path"));
				AICon->SetNewLocation(NavLoc.Location);
				return EBTNodeResult::Succeeded;
			}
			return EBTNodeResult::Failed;
		}
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Failed;
}

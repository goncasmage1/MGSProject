// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "AnAIController.h"
#include "BotTargetPoint.h"
#include "AudioManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"


AAnAIController::AAnAIController()
{
	//Initialize the behavior tree and blackboard components
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void AAnAIController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);

	//Get the possessed Pawn. If it's the AI Character we created
	//initialize it's blackboard and start it's corresponding behavior tree
	AMyAICharacter* AICharacter = Cast<AMyAICharacter>(pawn);
	if (AICharacter)
	{
		if (AICharacter->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*(AICharacter->BehaviorTree->BlackboardAsset));
			BehaviorComp->StartTree(*AICharacter->BehaviorTree);
		}
		BotTargetPoints = AICharacter->TargetPoints;
		SetSlowSpeed(true);
		SetState(EAIState::AI_Patrolling);
	}
}

void AAnAIController::SetState(EAIState NewState)
{
	if (BlackboardComp)
	{
		return BlackboardComp->SetValueAsEnum(CurrentStateKey, (uint8)NewState);
	}
}

EAIState AAnAIController::GetState() const
{
	if (BlackboardComp)
	{
		return (EAIState)BlackboardComp->GetValueAsEnum(CurrentStateKey);
	}
	return EAIState::AI_None;
}

void AAnAIController::SetSeenTarget(APawn* pawn)
{
	//Registers the Pawn that the AI has seen in the blackboard
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(TargetKey, pawn);
		ClearHelpBot();
	}
}

UObject * AAnAIController::GetSeenTarget()
{
	if (BlackboardComp)
	{
		return BlackboardComp->GetValueAsObject(TargetKey);
	}
	return nullptr;
}

void AAnAIController::ClearSeenTarget()
{
	if (BlackboardComp)
	{
		BlackboardComp->ClearValue(TargetKey);
	}
}

void AAnAIController::SetPlayerLocation(FVector Location)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(PlayerLocationKey, Location);
	}
}

FVector AAnAIController::GetPlayerLocation()
{
	if (BlackboardComp)
	{
		return BlackboardComp->GetValueAsVector(PlayerLocationKey);
	}
	return FVector::ZeroVector;
}

void AAnAIController::ClearPlayerLocation()
{
	if (BlackboardComp)
	{
		BlackboardComp->ClearValue(PlayerLocationKey);
	}
}

void AAnAIController::SetNewLocation(FVector Location)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(AILocationKey, Location);
	}
}

void AAnAIController::ClearNewLocation()
{
	if (BlackboardComp)
	{
		BlackboardComp->ClearValue(AILocationKey);
	}
}

void AAnAIController::SetHelpBot(APawn * pawn)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(HelpBotKey, pawn);
	}
}

UObject * AAnAIController::GetHelpBot()
{
	if (BlackboardComp)
	{
		return BlackboardComp->GetValueAsObject(HelpBotKey);
	}
	return nullptr;
}

void AAnAIController::ClearHelpBot()
{
	if (BlackboardComp)
	{
		BlackboardComp->ClearValue(HelpBotKey);
	}
}

void AAnAIController::SetTargetPoint(AActor * Actor)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(TargetPointKey, Actor);
	}
}

void AAnAIController::ClearTargetPoint()
{
	if (BlackboardComp)
	{
		BlackboardComp->ClearValue(TargetPointKey);
	}
}

void AAnAIController::SetHeardLocation(FVector Loc)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(HeardLocationKey, Loc);
	}
}

FVector AAnAIController::GetHeardLocation()
{
	if (BlackboardComp)
	{
		return BlackboardComp->GetValueAsVector(HeardLocationKey);
	}
	return FVector::ZeroVector;
}

void AAnAIController::ClearHeardLocation()
{
	if (BlackboardComp)
	{
		BlackboardComp->ClearValue(HeardLocationKey);
	}
}

void AAnAIController::SetSlowSpeed(bool bSlow)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsBool(SlowSpeedKey, bSlow);
	}
}

bool AAnAIController::GetSlowSpeed()
{
	if (BlackboardComp)
	{
		return BlackboardComp->GetValueAsBool(SlowSpeedKey);
	}
	return false;
}

void AAnAIController::NotifyAudioManager()
{
	AMyAICharacter* AICharacter = Cast<AMyAICharacter>(GetPawn());
	if (AICharacter) {
		AICharacter->AudioMan->RemoveEnemy(AICharacter);
	}
}




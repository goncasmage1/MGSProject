// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "MyAICharacter.h"
#include "AnAIController.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API AAnAIController : public AAIController
{
	GENERATED_BODY()
	
private:

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName CurrentStateKey = "CurrentState";

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName TargetKey = "Target";

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName AILocationKey = "AILocation";

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName PlayerLocationKey = "PlayerLocation";

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName HelpBotKey = "HelpBot";

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName TargetPointKey = "TargetPoint";

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName HeardLocationKey = "HeardLocation";

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName SlowSpeedKey = "SlowSpeed";

	/*Behavior Tree component reference*/
	UBehaviorTreeComponent* BehaviorComp;

	/*Blackboard comp ref*/
	UBlackboardComponent* BlackboardComp;

	//Array that contains all the BotTargetPoints
	TArray<class ABotTargetPoint*> BotTargetPoints;


public:

	/*Constructor*/
	AAnAIController();

	/*Executes right when the controller possess a Pawn*/
	virtual void OnPossess(APawn* Pawn) override;

	void SetState(EAIState NewState);
	EAIState GetState() const;

	void SetSeenTarget(APawn* Pawn);
	UObject* GetSeenTarget();
	void ClearSeenTarget();

	void SetNewLocation(FVector Location);
	void ClearNewLocation();

	void SetPlayerLocation(FVector Location);
	FVector GetPlayerLocation();
	void ClearPlayerLocation();

	void SetHelpBot(APawn* Pawn);
	UObject* GetHelpBot();
	void ClearHelpBot();

	void SetTargetPoint(AActor* Actor);
	void ClearTargetPoint();

	void SetHeardLocation(FVector Loc);
	FVector GetHeardLocation();
	void ClearHeardLocation();

	void SetSlowSpeed(bool bSlow);
	bool GetSlowSpeed();

	void NotifyAudioManager();

	
	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }

	FORCEINLINE TArray<class ABotTargetPoint*> GetAvailableTargetPoints() { return BotTargetPoints; }
	
	
};

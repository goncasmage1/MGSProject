// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "ChangePlayerSpeed.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API UChangePlayerSpeed : public UBTTaskNode
{
	GENERATED_BODY()
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	float DefaultSpeed = 600.f;
	float PatrolSpeed = 200.f;
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "WallCorner.h"
#include "CornerManager.generated.h"

USTRUCT(BlueprintType)
struct FCornerStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Corners", ExposeOnSpawn = true), Category = Radar)
		TArray<AWallCorner*> CornerArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Connect First to Last", ExposeOnSpawn = true), Category = Radar)
		bool bConnectFirstToLast;
};

UCLASS()
class CODEAI_API ACornerManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACornerManager();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Target Point", ExposeOnSpawn = true), Category = TargetPoints)
		TArray<FCornerStruct> Corners;
	
};

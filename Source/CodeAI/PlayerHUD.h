// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
	
	TArray<AActor*> RadarArray;

public:

	UPROPERTY(BlueprintReadWrite)
		FVector2D ScreenSize;
	UPROPERTY(BlueprintReadWrite)
		FVector2D RadarStartLocation = FVector2D(.85f, .2f);
	UPROPERTY(BlueprintReadWrite)
		float ObjectDistance = 10.f;
	UPROPERTY(BlueprintReadWrite)
		float RadarSize = 75;

	UFUNCTION(BlueprintCallable, Category = Minimap)
		void AddToRadar(AActor* NewActor);

	UFUNCTION(BlueprintCallable, Category = Minimap)
		void RemoveFromRadar(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = Minimap)
		FVector2D GetRadarCenterPosition();

	UFUNCTION(BlueprintCallable, Category = Minimap)
		FVector2D GetRadarDotPosition(FVector Location);

	UFUNCTION(BlueprintCallable, Category = Minimap)
		void DrawBorder();

	UFUNCTION(BlueprintCallable, Category = Minimap)
		void ContinueDrawHUD(int32 SizeX, int32 SizeY);
	
	
};

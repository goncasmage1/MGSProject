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

	TArray<AActor*> Corners;
	TArray<AActor*> EnemiesArray;
	class ACornerManager* Manager;

public:

	UPROPERTY(EditDefaultsOnly)
		UTexture2D* EnemySight;
	UPROPERTY(EditDefaultsOnly)
		FLinearColor EnemyAttackingColor;
	UPROPERTY(EditDefaultsOnly)
		FLinearColor EnemyInspectingColor;
	UPROPERTY(EditDefaultsOnly)
		FLinearColor EnemyPatrolingColor;
	UPROPERTY(EditDefaultsOnly)
		FLinearColor RadarBaseColor;

	UPROPERTY(BlueprintReadWrite)
		FVector2D RadarStartLocation = FVector2D(.85f, .2f);
	UPROPERTY(BlueprintReadWrite)
		float ObjectDistance = 10.f;
	UPROPERTY(BlueprintReadWrite)
		float RadarSize = 75;

	//Radar center position X and Y
	float RCPX, RCPY;

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = Minimap)
		void AddToRadar(AActor* NewActor);
	UFUNCTION(BlueprintCallable, Category = Minimap)
		void RemoveFromRadar(AActor* Actor);
	UFUNCTION(BlueprintCallable, Category = Minimap)
		FVector2D GetRadarDotPosition(FVector Location);
	UFUNCTION(BlueprintCallable, Category = Minimap)
		void FitToRadar(float &x, float &y);

	bool IsInRadar(float x, float y) const;

	//Draws the base upon which the rest of the icons will be drawn
	void DrawBase();
	//Draws the minimap border
	void DrawBorder();
	//Draw the enemies and the enemy cones
	void DrawEnemies();
	//Draw the cubes' and walls' borders
	void DrawCubes();

	UFUNCTION(BlueprintCallable, Category = Minimap)
		void ContinueDrawHUD(int32 SizeX, int32 SizeY);
		
};

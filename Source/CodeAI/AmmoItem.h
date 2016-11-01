// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameItem.h"
#include "AmmoItem.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API AAmmoItem : public AGameItem
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
		FName WeaponName;

	//Amount of (usable for stackable items)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 Quantity;

public:

	AAmmoItem();

	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UFUNCTION(BlueprintImplementableEvent, Category = ItemName)
		void UpdateText(bool bAmmoFull);

	FName GetWeaponName();

	int32 GetQuantity();

	void AddQuantity(int32 NewQuantity);
	
};


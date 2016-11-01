// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventoryGameItem.h"
#include "StackableGameItem.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API AStackableGameItem : public AInventoryGameItem
{
	GENERATED_BODY()

protected:

	//Amount of (usable for stackable items)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 Quantity;

	//Amount of (usable for stackable items)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Amount = 30.f;

	UFUNCTION()
		virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
public:

	int32 GetQuantity();

	void AddQuantity(int32 NewQuantity);
	UFUNCTION(BlueprintCallable, Category = Quantity)
		void DecreaseQuantity(int32 NewQuantity);
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameItem.h"
#include "InventoryGameItem.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API AInventoryGameItem : public AGameItem
{
	GENERATED_BODY()
	
protected:

	//The item's texture in the inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UTexture2D* ItemImage;

	//Determines whether this item is a gadget (displays in the left menu)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsGadget;

	UFUNCTION()
		virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

public:

	AInventoryGameItem();
	virtual void BeginPlay() override;

	virtual void UseItemPressed(bool bShouldHoldUp);
	virtual void UseItemReleased(bool bShouldStop);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Item)
		void MenuUse();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Weapon)
		void Equip();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Weapon)
		void Unequip();

	UTexture2D* GetItemTexture();
	
};

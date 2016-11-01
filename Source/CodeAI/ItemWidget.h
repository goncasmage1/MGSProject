// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ItemWidget.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category = UI)
		void Show();

	UFUNCTION(BlueprintImplementableEvent, Category = UI)
		void Hide();

	UFUNCTION(BlueprintImplementableEvent, Category = UI)
		void ChangeViewportPosition(FVector2D NewPosition);
	
	/*Holds a reference to the item texture*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UTexture2D* ItemTexture;

	/*Sets the item texture*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void SetItemAttributes(class AInventoryGameItem* Item);
		
	
};

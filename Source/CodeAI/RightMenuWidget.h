// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Blueprint/UserWidget.h"
#include "InventoryGameItem.h"
#include "RightMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API URightMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, Category = UI)
		void Show();

	/*Removes the widget from the viewport*/
	UFUNCTION(BlueprintImplementableEvent, Category = UI)
		void Hide();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<AInventoryGameItem*> ItemsArray;

	//class AMyPlayerController* MyPC;
	
	
};

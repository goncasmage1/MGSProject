// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "InventoryGameItem.h"
#include "PlayerUI.generated.h"

UCLASS()
class CODEAI_API UPlayerUI : public UUserWidget
{
	GENERATED_BODY()

public:

	//Shows the inventory
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void Show();

	//Shows the inventory
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void Hide();

	//Puts the widget on the viewport with the player's inventory as a parameter
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void BuildInventories();
	
	//Shows the inventory
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void ShowLeftInventory();

	//Toggles between the equipped item's visibility
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void ToogleShowLeftItem();

	//Toggles between the equipped item's visibility
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void UpdateCurrentLeftItem();

	//Hides the inventory
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void HideLeftInventory();

	//Plays the left menu's animation
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void PlayLeftMenuAnimation(bool bToTheRight);

	//Plays the right menu's animation
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void PlayRightMenuAnimation(bool bToTheRight);

	//The player's inventory array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<class AInventoryGameItem*> ItemsArray;
	
};

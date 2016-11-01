// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Blueprint/UserWidget.h"
#include "CodeAICharacter.h"
#include "LeftMenuWidget.generated.h"

UCLASS()
class CODEAI_API ULeftMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	//Puts the widget on the viewport with the player's inventory as a parameter
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void Build();
	
	//Shows the inventory
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void Show();

	//Toggles between the equipped item's visibility
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void ToogleShowItem();

	//Toggles between the equipped item's visibility
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void UpdateCurrentItem();

	//Hides the inventory
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void Hide();

	//Plays the menu's animation
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = UI)
		void PlayMenuAnimation(bool bToTheRight);
	
};

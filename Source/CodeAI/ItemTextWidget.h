// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ItemTextWidget.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API UItemTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category = Text)
		void SetText(FName ItemName);	
	
};

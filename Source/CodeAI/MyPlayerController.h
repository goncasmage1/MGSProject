// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

	uint8 bIsLeftMenuOpen : 1;
	uint8 bIsRightMenuOpen : 1;
	
public:

	virtual void Possess(APawn* InPawn) override;
	void ToogleLeftMenu();
	void ToogleCurrentItem();
	void UpdateItem();
	void MenuLeft();
	void MenuRight();

protected:

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UPlayerUI> PlayerUIBP;

private:

	class UPlayerUI* PlayerUIRef;
	class URightMenuWidget* RightMenuRef;
	
};

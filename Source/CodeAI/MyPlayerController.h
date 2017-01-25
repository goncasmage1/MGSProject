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
	uint8 bIsPaused : 1;
	uint8 bIsDead : 1;
	
public:

	virtual void Possess(APawn* InPawn) override;
	void ToogleLeftMenu();
	void ToogleCurrentItem();
	UFUNCTION(BlueprintCallable, Category = PauseMenu)
		void TooglePauseMenu();
	UFUNCTION(BlueprintCallable, Category = DeathMenu)
		void ShowDeathMenu();
	UFUNCTION(BlueprintCallable, Category = DeathMenu)
		void HideDeathMenu();
	void UpdateItem();
	void MenuLeft();
	void MenuRight();

protected:

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UPlayerUI> PlayerUIBP;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UPauseMenuWidget> PauseMenuBP;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UDeathMenuWidget> DeathMenuBP;

private:

	class UPlayerUI* PlayerUIRef;
	class UPauseMenuWidget* PauseMenuRef;
	class UDeathMenuWidget* DeathMenuRef;
	
};

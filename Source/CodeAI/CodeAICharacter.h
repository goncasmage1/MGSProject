// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "CodeAICharacter.generated.h"

UCLASS(config = Game)
class ACodeAICharacter : public ACharacter
{
	GENERATED_BODY()

		/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* LeftBoom;
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* LeftCamera;
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* RightBoom;
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* RightCamera;

public:

	ACodeAICharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/*		COVER		*/
	//Handles the logic to determine whether or not the player is in cover and
	//from which direction the cover was triggered
	void HandleCoverLogic();
	//Handles variable changes when the player leaves cover
	void OnEndCover();
	//Handles the line traces to check whether or not the player is at the edge of cover
	void HandleCoverLineTrace();
	//"Sub-function" executed by the HandleCoverLineTrace() function
	void HandleForwardCover();
	//"Sub-function" executed by the HandleCoverLineTrace() function
	void HandleRightCover();
	//Toggles between the top-down and front cameras
	void ToggleCamera(bool bDefaultCamera, bool bLeftCamera = false);	
	/*		COVER		*/

	/*		ITEMS		*/
	//The array that stores the player's items
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
		TArray<class AInventoryGameItem*> InventoryArray;

	bool AddItem(class AGameItem* Item);
	bool AddNewItem(class AInventoryGameItem* NewItem);
	bool AddAmmoItem(class AAmmoItem* AmmoItem);
	bool AddWeapon(class AWeaponItem* WeaponItem);
	bool AddStackable(class AStackableGameItem* StackableItem);
	void EquipWeapon(class AWeaponItem* WeaponItem);
	//Called from the Menu widget to set the new index
	UFUNCTION(BlueprintCallable, Category = Item)
		void SetEquippedIndex(int32 Index);
	//Called to set the new index
	void SetIndex(int32 Index);
	/*		ITEMS		*/

	/*		HEALTH		*/
	//The player's health on a scale of 0 to 1
	UPROPERTY(BlueprintReadOnly, Category = Health)
		float HUDHealth;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser);
	UFUNCTION(BlueprintCallable, Category = Health)
		virtual void AddHealth(float Regen);
	/*		HEALTH		*/

	/*		MOVEMENT		*/
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;
	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;
	//Called when the player's movement is being blocked by an actor
	virtual void MoveBlockedBy(const FHitResult & Impact) override;

	/*		MOVEMENT		*/

	/*		NOISE		*/
	/*Plays a sound in-game and reports it to entities who have a UPawnSensingComponent (AI)*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UPawnNoiseEmitterComponent* PawnNoiseEmitterComp;

	UFUNCTION(BlueprintCallable, Category = Sound)
		void ReportNoise(USoundBase* SoundToPlay, float Volume, bool bShouldBeLouder);
	/*		NOISE		*/

private:

	//Indicates the index of the currently equipped item
	int32 EquippedIndex;
	//Used to keep track of menu navigation
	int32 ExtraIndex;

	/*		FLOATS		*/
	//Indicates the factor at which the player's speed decreases when walking
	float WalkSpeedDecrease;
	//Indicates the factor at which the player's speed decreases when in cover
	UPROPERTY(EditAnywhere, Category = Cover)
		float CoverSpeedDecrease;
	//Registers the character's forward movement on tick
	float ForwardMov;
	//Registers the character's right movement on tick
	float RightMov;
	//Registers the amount of movement that is "illegal" while in cover
	float NoMov;
	//Registers the X component of the mouse control
	float XRate;
	//Registers the Y component of the mouse control
	float YRate;
	/*Registers for how long the player has to hold down the menu button
	in order for the menu to pop up*/
	float MenuTimer;
	UPROPERTY(EditDefaultsOnly)
		float MenuHeldDownTime;
	//The player's health
	float Health;
	//The player's max health
	UPROPERTY(EditDefaultsOnly)
		float MaxHealth;
	//Amount of damage to apply to the enemy
	float Damage;
	//Indicates the distance to the side of the player used to make
	//the line trace from - HandleCoverLineTrace() function
	UPROPERTY(EditAnywhere, Category = Cover)
		float CoverPeakDistance;
	/*		FLOATS		*/

	/*		BOOLEANS		*/
	//Determines whether the player is sprinting or not
	uint8 bIsWalking : 1;
	//Determines whether the player is in cover or not
	uint8 bIsInCover : 1;
	//Determines when to exit cover from a forward movement
	uint8 bShouldBeInCoverForward : 1;
	//Determines when to exit cover from a right movement
	uint8 bShouldBeInCoverRight : 1;
	//Determines which direction the NoMov applies to
	uint8 bNoMovForward : 1;

	//Whether or not the player has a pistol equipped
	uint8 bPistolEquipped : 1;
	//Whether or not the player has a rifle equipped
	uint8 bRifleEquipped : 1;
	//Determines whether an item is equipped or not
	uint8 bItemEquipped : 1;
	//Determines whether the player should add the picked up item to the inventory
	uint8 bShouldAddItem : 1;

	//Determines whether the left menu is open or closed
	uint8 bLeftMenuOpen : 1;
	//Determines whether the right menu is open or closed
	uint8 bRightMenuOpen : 1;
	//Determines whether the player pressed the left menu button
	uint8 bLeftMenuPressed : 1;
	//Determines whether the player is holding down the menu button
	uint8 bHeldDownMenu : 1;
	//Determines whether the player can navigate through the inventory
	uint8 bAllowNavigation : 1;
	//Determines whether the player can move or not
	uint8 bAllowMovement : 1;
	/*		BOOLEANS		*/

protected:

	/*		HANDLE OF INPUT		*/
	virtual void SetupPlayerInputComponent(class UInputComponent* inputComponent) override;
	/** Called for forwards/backward input */
	void MoveForward(float Value);
	/** Called for side to side input */
	void MoveRight(float Value);
	//Handles the player's input over the menu
	void HandleMenuInput(float Value);
	//Switches between firing a weapon and moving regurarly
	void ToogleCharacterControls(bool bAllow);
	//Called when the player presses the left menu button
	void PressedLeftMenu();
	//Called when the player holds the left menu button
	void ShowLeftMenu();
	//Called when the player releases the left menu button
	void HideLeftMenu();
	//Called when the player presses the right menu button
	void PressedRightMenu();
	//Called when the player holds the right menu button
	void ShowRightMenu();
	//Called when the player releases the right menu button
	void HideRightMenu();
	//Called when the action button is pressed
	void ActionPressed();
	//Called when the action button is released
	void ActionReleased();
	//Called when the use button is pressed
	void UsePressed();
	//Called when the use button is released
	void UseReleased();
	//Called when the reload button is pressed
	void ReloadPressed();
	//Called when the reload button is released
	void ReloadReleased();
	/*		HANDLE OF INPUT		*/

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);
	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

	void MouseTurnX(float Rate);
	void MouseTurnY(float Rate);
	//Lowers the player's speed
	void Walk();
	//Resumes running by the player
	void StopWalking();
	//Performs a line trace and returns whether the hit result was a blocking hit
	bool InCoverLeftLineTrace(bool bToTheLeft, bool bForward);

	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundBase* MenuSound;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundBase* KnockingSound;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundBase* PlayerHurt_1;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundBase* PlayerHurt_2;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundBase* PlayerDead;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundBase* PlayerCover;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundBase* InventoryOpen;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundBase* ItemEquip;
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundBase* ItemUnequip;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	//Returns whether or not a pistol is equipped
	UFUNCTION(BlueprintCallable, Category = Weapon)
		FORCEINLINE bool IsPistolEquipped() const { return bPistolEquipped; }
	//Returns whether or not a rifle is equipped
	UFUNCTION(BlueprintCallable, Category = Weapon)
		FORCEINLINE bool IsRifleEquipped() const { return bRifleEquipped; }
	//Returns whether or not the player is walking
	UFUNCTION(BlueprintCallable, Category = Movement)
		FORCEINLINE	bool GetIsWalking() const { return bIsWalking; }
	//Returns whether or not the player is in cover
	UFUNCTION(BlueprintCallable, Category = Cover)
		FORCEINLINE bool InCover() const { return bIsInCover; }
	//Returns the index of the currently equipped item
	UFUNCTION(BlueprintCallable, Category = Inventory)
		FORCEINLINE int32 GetEquippedIndex() const { return EquippedIndex; }
	//Returns the player's health
	UFUNCTION(BlueprintCallable, Category = Health)
		FORCEINLINE float GetHealth() const { return Health; }
	UFUNCTION(BlueprintCallable, Category = Health)
		FORCEINLINE bool FullHealth() const { return Health == MaxHealth; }
	//Returns the player's damage
	FORCEINLINE float GetDamage() const { return Damage; }
	//Returns the player's inventory
	FORCEINLINE TArray<class AInventoryGameItem*> GetInventory() const { return InventoryArray; }
	 
};


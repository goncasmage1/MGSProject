// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "CodeAICharacter.generated.h"

UCLASS(config = Game)
class ACodeAICharacter : public ACharacter
{
	GENERATED_BODY()

private:

	/******************************************
	*		COMPONENTS
	******************************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TransitionCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* TopDownBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TopDownCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* LeftBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* LeftCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* RightBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* RightCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* DeathBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* DeathCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FPPCamera;

	//Sounds
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

	//List of animations to pick from randomly
	UPROPERTY(EditDefaultsOnly)
		TArray<UAnimationAsset*> DeathAnimations;

	//List of animations to pick from randomly
	UPROPERTY(EditDefaultsOnly)
		TArray<UAnimationAsset*> ProneDeathAnimations;

	UPROPERTY(EditDefaultsOnly)
		TArray<UAnimSequence*> StaggerAnimations;

public:

	ACodeAICharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/******************************************
	*		COVER
	******************************************/

	/**
	* Performs a line trace and returns whether the hit result was a blocking hit
	* @param bToTheLeft Indicates the side from which to start the raycast
	* @param bForward Indicates whether the raycast will be directed in the x or y axis
	*/
	bool CoverLineTrace(bool bLeft, bool bForward);

	//Determines whether the player should be in cover
	void HandleCoverLogic();

	//Handles the line traces to check whether or not the player is at the edge of cover
	void HandleCoverLineTrace();

	//"Sub-function" executed by the HandleCoverLineTrace() function
	void HandleForwardCover();

	//"Sub-function" executed by the HandleCoverLineTrace() function
	void HandleRightCover();

	//Handles variable changes when the player leaves cover
	void OnEndCover();

	//Toggles between the top-down and front cameras
	void ToggleCamera(bool bDefaultCamera, bool bLeftCamera = false);

	UFUNCTION(BlueprintCallable, Category = Cover)
		FORCEINLINE bool InCover() const { return bIsInCover; }


	/******************************************
	*		ITEMS
	******************************************/

	//The array that stores the player's items
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
		TArray<class AInventoryGameItem*> InventoryArray;

	/**
	* Called when the player overlaps a GameItem.
	* Will call one of the mroe specific AddItem functions.
	*/
	bool AddItem(class AGameItem* Item);

	//Add a generic InventoryItem
	bool AddNewItem(class AInventoryGameItem* NewItem);

	//Add an AmmoItem
	bool AddAmmoItem(class AAmmoItem* AmmoItem);

	//Add a WeaponItem
	bool AddWeapon(class AWeaponItem* WeaponItem);

	//Add a Stackable Item
	bool AddStackable(class AStackableGameItem* StackableItem);

	//Equip the selected weapon
	void EquipWeapon(class AWeaponItem* WeaponItem);

	//Called from the Menu widget to set the new index according to the equipped item
	UFUNCTION(BlueprintCallable, Category = Item)
		void SetEquippedIndex(int32 Index);

	//Sets the player's item index every tiem the player moves around the menu
	void SetIndex(int32 Index);
	
	UFUNCTION(BlueprintCallable, Category = Weapon)
		FORCEINLINE bool IsPistolEquipped() const { return bPistolEquipped; }

	UFUNCTION(BlueprintCallable, Category = Weapon)
		FORCEINLINE bool IsRifleEquipped() const { return bRifleEquipped; }

	UFUNCTION(BlueprintCallable, Category = Inventory)
		FORCEINLINE int32 GetEquippedIndex() const { return EquippedIndex; }

	FORCEINLINE TArray<class AInventoryGameItem*> GetInventory() const { return InventoryArray; }

	/******************************************
	*		HEALTH
	******************************************/

	//The player's health on a scale of 0 to 1
	UPROPERTY(BlueprintReadOnly, Category = Health)
		float HUDHealth;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser);

	//Called when the player dies
	void OnDeath();

	//Called inside the OnDeath function
	void SwitchToDeathCamera();

	UFUNCTION(BlueprintCallable, Category = Health)
		virtual void AddHealth(float Regen);

	//Adds an enemy to the list of enemies to notify about the player's death
	void AddEnemy(class AMyAICharacter* NewEnemy);

	//Removes an enemy from the list of enemies to notify about the player's death
	void RemoveEnemy(class AMyAICharacter* NewEnemy);

	UFUNCTION(BlueprintCallable, Category = Health)
		FORCEINLINE float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = Health)
		FORCEINLINE bool FullHealth() const { return Health == MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = Health)
		FORCEINLINE bool IsDead() const { return bIsDead; }


	/******************************************
	*		MOVEMENT
	******************************************/

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;
	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	//Called when the player's movement is being blocked by an actor
	virtual void MoveBlockedBy(const FHitResult & Impact) override;

	UFUNCTION(BlueprintCallable, Category = Movement)
		FORCEINLINE	bool GetIsWalking() const { return bIsWalking; }

	UFUNCTION(BlueprintCallable, Category = Movement)
		FORCEINLINE	bool GetIsCrouching() const { return bIsCrouching; }

	UFUNCTION(BlueprintCallable, Category = Movement)
		FORCEINLINE	bool GetIsProne() const { return bIsProne; }


	/******************************************
	*		NOISE
	******************************************/

	//Used to notify the AI of noises made
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UPawnNoiseEmitterComponent* PawnNoiseEmitterComp;

	/*Plays a sound in-game and reports it to entities who have a UPawnSensingComponent (AI)*/
	UFUNCTION(BlueprintCallable, Category = Sound)
		void ReportNoise(USoundBase* SoundToPlay, float Volume);

	//Used to notify all the bots that there was a loud noise
	void NotifyLoudNoise(FVector Loc);


private:

	/******************************************
	*		ITEMS
	******************************************/

	//Indicates the index of the currently equipped item
	int32 EquippedIndex;
	//Indicates the index of the previously equipped item
	int32 PreviousIndex;
	//Used to keep track of menu navigation
	int32 ExtraIndex;


	/******************************************
	*		MOVEMENT
	******************************************/

	//Indicates the factor at which the player's speed decreases when walking
	float WalkSpeedDecrease;
	//Registers the X component of the mouse control
	float XRate;
	//Registers the Y component of the mouse control
	float YRate;

	//Used to time the crouch movement
	FTimerHandle CrouchHandle;

	//Keeps track of the player's movement while crouching
	float HorizontalCrouchMov;
	float VerticalCrouchMov;

	//The amount of time to wait before the player goes prone after crouching
	UPROPERTY(EditAnywhere)
		float CrouchWait;

	//Movement speed ratio when prone
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.2", ClampMax = "6.0", UIMin = "0.2", UIMax = "6.0"))
		float ProneMovement;

	//Used for playing animations
	FTimerHandle AnimationHandle;

	UPROPERTY(EditAnywhere)
		UAnimSequence* ProneMontage;
	UPROPERTY(EditAnywhere)
		float ProneMontageLength;

	UPROPERTY(EditAnywhere)
		UAnimSequence* CrouchMontage;
	UPROPERTY(EditAnywhere)
		float CrouchMontageLength;

	//Determines whether the player is sprinting or not
	uint8 bIsWalking : 1;

	//Determines whether the player can move or not
	uint8 bAllowMovement : 1;

	//Determines whether the player is crouching
	uint8 bIsCrouching : 1;

	//Determines whether the player is pressing the crouching button
	uint8 bIsCrouchPressed : 1;

	//Determines whether the player is prone
	uint8 bIsProne: 1;

	//Determines whether the player is staggering
	uint8 bIsStaggering : 1;

	//A reference to the world camera currently being used
	UCameraComponent* TemporaryCamera;


	/******************************************
	*		COVER
	******************************************/

	//Indicates the factor at which the player's speed decreases when in cover
	UPROPERTY(EditAnywhere, Category = Cover)
		float CoverSpeedDecrease;

	/**
	* Indicates the distance to the side of the player used to make
	* the line trace from - HandleCoverLineTrace() function
	*/
	UPROPERTY(EditAnywhere, Category = Cover)
		float CoverPeakDistance;

	//Registers the character's forward movement on tick
	float ForwardMov;
	//Registers the character's right movement on tick
	float RightMov;
	//Registers the amount of movement that is "illegal" while in cover
	float NoMov;

	//Determines whether the player is in cover
	uint8 bIsInCover : 1;
	//Determines when to exit cover from a forward movement
	uint8 bShouldBeInCoverForward : 1;
	//Determines when to exit cover from a right movement
	uint8 bShouldBeInCoverRight : 1;
	//Determines which direction the NoMov applies to
	uint8 bNoMovForward : 1;


	/******************************************
	*		MENU
	******************************************/

	/**
	* The amount of time the player has to hold down the menu button
	* in order for the menu to pop up
	*/
	float MenuTimer;
	UPROPERTY(EditAnywhere)
		float MenuHeldDownTime;

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
	//Determines whether the player is using a First Person Camera
	uint8 bUsingFPP : 1;

	/******************************************
	*		ENEMIES
	******************************************/

	//List of enemies to notify when player dies (they need to go back to patroling)
	TArray<class AMyAICharacter*> AttackingEnemies;

	/**
	* List of enemies to notify (all of them) when the player makes a loud noise
	* i.e. firing an unsilenced gun
	*/
	TArray<class AMyAICharacter*> AllEnemies;


	/******************************************
	*		HEALTH
	******************************************/

	float Health;
	UPROPERTY(EditAnywhere)
		float MaxHealth;

	uint8 bIsDead : 1;

	/******************************************
	*		ITEMS
	******************************************/

	//Whether the player has a pistol equipped
	uint8 bPistolEquipped : 1;
	//Whether the player has a rifle equipped
	uint8 bRifleEquipped : 1;
	//Determines whether an item is equipped or not
	uint8 bItemEquipped : 1;
	//Determines whether the player should add the picked up item to the inventory
	uint8 bShouldAddItem : 1;


	/******************************************
	*		OCTOCAMO
	******************************************/

	//Time until the octocamo scans the environment
	float ScanWaitTime;
	float TextureTransitionSpeed;

	//Indicates whether to consider a texture transition
	uint8 bOctoCamoTransition;
	uint8 bTransitionTextureIndex;
	/*Indicates whether to change between main texture and transition texture
	or between two transition textures*/
	uint8 bFirstChange;

	FTimerHandle OctoCamoHandle;

	TArray<UMaterialInstanceDynamic*> MIDs;

	UTexture* TransitionTexture;
	UTexture* PreviousTexture;

protected:

	/******************************************
	*		INPUT
	******************************************/

	//Handles the functions calls when a certain input is pressed
	virtual void SetupPlayerInputComponent(class UInputComponent* inputComponent) override;

	//Handles the player's input over the menu
	void HandleMenuInput(float Value);

	/**
	* Switches controls between firing a weapon and moving
	* @param bAllow Indicates whether the player can move, otherwise, firing control scheme applies
	*/
	void ToogleCharacterControls(bool bAllow);

	//Setups the material to allow OctoCamo functionality
	void SetupMaterials();

	void ApplyMaterialTransition();
	void HandleOctoCamoTransition(float DeltaTime);
	//Checks if the necessary conditions for OctoCamo to start scanning are met
	void CheckOctoCamo();
	bool TextureLineTrace(bool bStanding);

	//Handles the player's rotation when using a weapon
	void HandlePlayerRotation();

	void PressedLeftMenu();
	void ShowLeftMenu();
	void HideLeftMenu();

	void PressedRightMenu();
	void ShowRightMenu();
	void HideRightMenu();

	void ActionPressed();
	void ActionReleased();

	void UsePressed();
	void UseReleased();

	void ReloadPressed();
	void ReloadReleased();

	void FPPPressed();
	void FPPReleased();

	void PausePressed();

	void CrouchPressed();
	void CrouchReleased();

	//Updates the player's rotation (used when crouching)
	void UpdateRotation(float NewHor, float NewVer);

	//Make the proper changes to allow crouching to prone functionality
	void PrepareProne();

	//Change the variables to start prone
	void StartProne();

	//When the prone animation is finished
	void FinishProne();

	//Make the proper changes to allow prone to crouching functionality
	void PrepareCrouch();

	//Handle the staggering mechanic
	void HandleStagger();

	//When the stagger mechanic has finished
	void FinishStagger();

	//Lowers the player's speed
	void Walk();

	//Resumes running
	void StopWalking();

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

	/** Called for forwards/backward input */
	void MoveForward(float Value);
	//Handle the vertical portion of crouching
	void HandleVerticalCrouch(float Value);
	//Handle the prone movement
	void HandleProneMovement(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);
	//Handle the horizontal portion of crouching
	void HandleHorizontalCrouch(float Value);
	//Handle the prone rotation
	void HandleProneRotation(float Value);

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCamera() const { return TopDownCamera; }
	 
};


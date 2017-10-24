// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CodeAI.h"
#include "CodeAICharacter.h"
#include "MyAICharacter.h"
#include "MGSCube.h"
#include "MyPlayerController.h"
#include "InventoryGameItem.h"
#include "WeaponItem.h"
#include "StackableGameItem.h"
#include "AmmoItem.h"
#include "NightVisionGoogles.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

ACodeAICharacter::ACodeAICharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	TransitionCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TransitionCamera"));
	TransitionCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	TransitionCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	TopDownBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Top Down Boom"));
	TopDownBoom->SetupAttachment(RootComponent);
	TopDownBoom->bUsePawnControlRotation = true;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Top Down Camera"));
	TopDownCamera->SetupAttachment(TopDownBoom, USpringArmComponent::SocketName);

	LeftBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Left Boom"));
	LeftBoom->SetupAttachment(RootComponent);
	LeftBoom->bUsePawnControlRotation = true;

	LeftCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Left Camera"));
	LeftCamera->SetupAttachment(LeftBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation

	RightBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Right Boom"));
	RightBoom->SetupAttachment(RootComponent);
	LeftBoom->bUsePawnControlRotation = true;

	RightCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Right Camera"));
	RightCamera->SetupAttachment(RightBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation

	DeathBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Death Boom"));
	DeathBoom->SetupAttachment(RootComponent);

	DeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Death Camera"));
	DeathCamera->SetupAttachment(DeathBoom, USpringArmComponent::SocketName);

	FPPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPP Camera"));
	FPPCamera->SetupAttachment(RootComponent);

   // Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
   // are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	PawnNoiseEmitterComp = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("PawnNoiseEmitterComp"));

	bIsWalking = false;
	bIsInCover = false;
	bShouldBeInCoverForward = false;
	bShouldBeInCoverRight = false;
	bNoMovForward = false;

	bPistolEquipped = false;
	bRifleEquipped = false;
	bLeftMenuOpen = false;
	bRightMenuOpen = false;
	bLeftMenuPressed = false;
	bHeldDownMenu = false;
	bItemEquipped = false;
	bAllowNavigation = true;
	bUsingFPP = false;

	bShouldAddItem = true;
	bAllowMovement = true;
	bIsUsingItem = false;
	bIsCrouching = false;
	bIsCrouchPressed = false;
	bIsProne = false;
	bIsStaggering = false;
	bIsDead = false;
	bIsInCameraSection = false;

	bOctoCamoTransition = false;
	bFirstChange = true;

	EquippedIndex = ExtraIndex = 0;
	PreviousIndex = -1;

	WalkSpeedDecrease = 3.f;
	PrevYaw = -1.f;
	RotationDeadAngle = 3.f;
	CoverSpeedDecrease = 2.5f;
	ForwardMov = RightMov = NoMov = 0.f;
	HorizontalCrouchMov = VerticalCrouchMov = 0.f;
	XRate = YRate = 0.f;
	MenuHeldDownTime = 0.2f;
	CrouchWait = 0.2f;
	ProneMovement = 0.35f;
	CoverPeakDistance = 60.f;

	ScanWaitTime = 1.5f;
	TextureTransitionSpeed = 2.f;
	bTransitionTextureIndex = 0;

	MaxHealth = 100.f;
	Health = MaxHealth;
	HUDHealth = 1.f;
}

void ACodeAICharacter::BeginPlay()
{
	Super::BeginPlay();

	TransitionCamera->SetWorldTransform(TopDownCamera->GetComponentTransform());

	GetCharacterMovement()->bOrientRotationToMovement = true;

	for (TActorIterator<AInventoryGameItem> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		//Add the NONE item to the player's inventory
		if (ActorItr->GetItemName() == FName("None")) {
			InventoryArray.Add(*ActorItr);
		}
	}
	for (TActorIterator<AMyAICharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AllEnemies.Add(*ActorItr);
	}

	SetupMaterials();

	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
	if (MyPC)
	{
		MyPC->bShowMouseCursor = true;
		FInputModeGameAndUI NewInput;
		MyPC->SetInputMode(NewInput);
	}
}

void ACodeAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Cover logic
	if (bIsInCover) {
		HandleCoverLogic();
		HandleCoverLineTrace();
	}

	//OctoCamo
	if (!bOctoCamoTransition)
	{
		CheckOctoCamo();
	}
	else
	{
		HandleOctoCamoTransition(DeltaTime);
	}

	//Player Rotation while firing
	if (!bAllowMovement) {
		if (!bUsingFPP)
		{
			HandlePlayerRotation();
		}
		else
		{
			AddControllerYawInput(XRate);
		}
	}
}

void ACodeAICharacter::SetupMaterials()
{
	MIDs.Add(GetMesh()->CreateAndSetMaterialInstanceDynamic(0));
	MIDs.Add(GetMesh()->CreateAndSetMaterialInstanceDynamic(1));
}

void ACodeAICharacter::ApplyMaterialTransition()
{
	TextureLineTrace(bIsInCover);
	//If the newly scanned texture is different, apply texture changes;
	if (PreviousTexture != TransitionTexture)
	{
		bOctoCamoTransition = true;
		if (bTransitionTextureIndex)
		{
			bTransitionTextureIndex--;
			MIDs[0]->SetTextureParameterValue(FName("Transition Texture 2"), TransitionTexture);
		}
		else
		{
			bTransitionTextureIndex++;
			MIDs[0]->SetTextureParameterValue(FName("Transition Texture 1"), TransitionTexture);
		}
	}
}

void ACodeAICharacter::HandleOctoCamoTransition(float DeltaTime)
{
	//Set the alpha parameter's name
	FName AlphaName = bFirstChange ? FName("Main Texture Alpha") : FName("Transition Texture Alpha");

	float Alpha;
	MIDs[0]->GetScalarParameterValue(AlphaName, Alpha);
	float NewAlpha = bTransitionTextureIndex ? Alpha + (TextureTransitionSpeed * DeltaTime) : Alpha - (TextureTransitionSpeed * DeltaTime);
	if (bTransitionTextureIndex && NewAlpha > 1.f)
	{
		NewAlpha = 1.f;
		bOctoCamoTransition = false;
		bFirstChange = false;
	}
	else if (!bTransitionTextureIndex && NewAlpha < 0.f)
	{
		NewAlpha = 0.f;
		bOctoCamoTransition = false;
		bFirstChange = false;
	} 
	MIDs[0]->SetScalarParameterValue(AlphaName, NewAlpha);
}

void ACodeAICharacter::CheckOctoCamo()
{
	/*Start timer when player stays still while in cover or prone
	Stop timer when player moves or is not in cover or prone*/
	if ((bIsInCover || bIsProne) && GetVelocity().Size() == 0 && !(GetWorld()->GetTimerManager().IsTimerActive(OctoCamoHandle)))
	{
		GetWorld()->GetTimerManager().SetTimer(OctoCamoHandle, this, &ACodeAICharacter::ApplyMaterialTransition, ScanWaitTime);
	}
	else if (!((bIsInCover || bIsProne) && GetVelocity().Size() == 0))
	{
		GetWorld()->GetTimerManager().ClearTimer(OctoCamoHandle);
	}
}

bool ACodeAICharacter::TextureLineTrace(bool bStanding)
{
	FVector Start, End;
	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	PreviousTexture = TransitionTexture;

	Start = GetActorLocation();

	/*If the player is standing, trace to the back of the player (player in cover),
	otherwise, trace down (player is prone)*/
	End = bStanding ? Start - (GetActorForwardVector() * 100.f) : Start - (GetActorUpVector() * 100.f);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic, QueryParams)) {
		UStaticMeshComponent* TraceMesh = Cast<UStaticMeshComponent>(Hit.GetComponent());
		if (TraceMesh)
		{
			TraceMesh->GetMaterials()[0]->GetTextureParameterValue(FName("OctoCamo Texture"), TransitionTexture);
			return true;
		}
	}

	return false;
}

void ACodeAICharacter::ToogleNVGMaterial(bool bActivateNVG)
{
	if (bActivateNVG)
	{
		MIDs[0]->SetScalarParameterValue(FName("NVG"), 0);
	}
	else
	{
		MIDs[0]->SetScalarParameterValue(FName("NVG"), 1);
	}
}

void ACodeAICharacter::HandlePlayerRotation()
{
	float MousePosX;
	float MousePosY;

	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());

	if (MyPC->GetMousePosition(MousePosX, MousePosY))
	{
		//Convert character location to 2D space
		FVector2D CharPos2D;
		MyPC->ProjectWorldLocationToScreen(GetActorLocation(), CharPos2D);

		FVector2D Result;
		Result.X = -(MousePosY - CharPos2D.Y);
		Result.Y = MousePosX - CharPos2D.X;

		float Angle = FMath::RadiansToDegrees(FMath::Acos(Result.X / Result.Size()));
		if (Result.Y < 0.f)
		{
			Angle = 360.f - Angle;
		}

		FRotator NewRot(0.f, Angle, 0.f);

		SetActorRotation(NewRot);
	}
}

void ACodeAICharacter::HandleCoverLogic()
{
	//If the player enters cover from a forward position
	if (ForwardMov != 0.0f && !bShouldBeInCoverForward && !bShouldBeInCoverRight && NoMov == 0) {
		bShouldBeInCoverForward = true;
		return;
	}
	//If the player enters cover from a right position
	else if (RightMov != 0.0f && !bShouldBeInCoverForward && !bShouldBeInCoverRight && NoMov == 0) {
		bShouldBeInCoverRight = true;
		return;
	}
	//If the player exits cover from a forward position
	else if (ForwardMov == 0.0f && bShouldBeInCoverForward) {
		OnEndCover();
		return;
	}
	//If the player exits cover from a right position
	else if (RightMov == 0.0f && bShouldBeInCoverRight) {
		OnEndCover();
		return;
	}
}

void ACodeAICharacter::OnEndCover()
{
	bShouldBeInCoverForward = false;
	bShouldBeInCoverRight = false;
	bIsInCover = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed *= CoverSpeedDecrease;
	NoMov = 0.f;
	if (!TopDownCamera->IsActive()) {
		ToggleCamera(true, false);
	}
}

void ACodeAICharacter::HandleCoverLineTrace()
{
	//If the cover was triggered by a forward movement
	if (bShouldBeInCoverForward) {
		HandleForwardCover();
	}
	//If the cover was triggered by a right movement
	else if (bShouldBeInCoverRight) {
		HandleRightCover();
	}
}

void ACodeAICharacter::HandleForwardCover()
{
	if (ForwardMov != 0.f && RightMov > 0.f) {
		/*When the line trace fails, the player reached the end of the cover
		and should restrain from moving in that direction and also switch camera view*/
		if (!CoverLineTrace(true, true)) {
			NoMov = RightMov;
			bNoMovForward = false;
			if (TopDownCamera->IsActive()) {
				if (ForwardMov > 0.f) {
					ToggleCamera(false, true);
				}
				else {
					ToggleCamera(false, false);
				}
			}
		}
	}
	else if (ForwardMov != 0.f && RightMov < 0.f) {
		/*When the line trace fails, the player reached the end of the cover
		and should restrain from moving in that direction and also switch camera view*/
		if (!CoverLineTrace(false, true)) {
			NoMov = RightMov;
			bNoMovForward = false;
			if (TopDownCamera->IsActive()) {
				if (ForwardMov > 0.f) {
					ToggleCamera(false, false);
				}
				else {
					ToggleCamera(false, true);
				}
			}
		}
	}
}

void ACodeAICharacter::HandleRightCover()
{
	if (RightMov != 0.f && ForwardMov > 0.f) {
		/*When the line trace fails, the player reached the end of the cover
		and should restrain from moving in that direction and also switch camera view*/
		if (!CoverLineTrace(true, false)) {
			NoMov = ForwardMov;
			bNoMovForward = true;
			if (TopDownCamera->IsActive()) {
				if (RightMov > 0.f) {
					ToggleCamera(false, false);
				}
				else {
					ToggleCamera(false, true);
				}
			}
		}
	}
	else if (RightMov != 0.f && ForwardMov < 0.f) {
		/*When the line trace fails, the player reached the end of the cover
		and should restrain from moving in that direction and also switch camera view*/
		if (!CoverLineTrace(false, false)) {
			NoMov = ForwardMov;
			bNoMovForward = true;
			if (TopDownCamera->IsActive()) {
				if (RightMov > 0.f) {
					ToggleCamera(false, true);
				}
				else {
					ToggleCamera(false, false);
				}
			}
		}
	}
}

void ACodeAICharacter::PressedLeftMenu()
{
	if (!bIsDead) {
		bLeftMenuPressed = true;
		GetWorld()->GetTimerManager().SetTimer(MenuTimerHandle, this, &ACodeAICharacter::ShowLeftMenu, MenuHeldDownTime, false);
	}
}

void ACodeAICharacter::ShowLeftMenu()
{
	bLeftMenuOpen = true;
	if (InventoryOpen) {
		UGameplayStatics::PlaySound2D(GetWorld(), InventoryOpen);
	}
	PreviousIndex = EquippedIndex;
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
	if (MyPC) {
		MyPC->ToogleLeftMenu();
	}
}

void ACodeAICharacter::HideLeftMenu()
{
	bLeftMenuPressed = false;
	bAllowNavigation = true;

	GetWorld()->GetTimerManager().ClearTimer(MenuTimerHandle);

	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());

	//If the menu was opened (button was held)
	if (bLeftMenuOpen)
	{
		bLeftMenuOpen = false;
		if (MyPC)
		{
			MyPC->ToogleLeftMenu();
		}
		if (EquippedIndex > 0)
		{
			bItemEquipped = true;
		}
	}

	//If the item was toggled (button was simply pressed)
	else if (EquippedIndex > 0)
	{
		if (bItemEquipped)
		{
			bItemEquipped = false;

			//Play sound
			if (ItemUnequip)
			{
				UGameplayStatics::PlaySound2D(GetWorld(), ItemUnequip);
			}

			InventoryArray[EquippedIndex]->Unequip();
		}
		else
		{
			bItemEquipped = true;

			//Play sound
			if (ItemEquip)
			{
				UGameplayStatics::PlaySound2D(GetWorld(), ItemEquip);
			}

			InventoryArray[EquippedIndex]->Equip();

		}
		if (MyPC)
		{
			MyPC->ToogleCurrentItem();
		}
	}
}

void ACodeAICharacter::PressedRightMenu()
{
	if (!bIsDead) {

	}
}

void ACodeAICharacter::ShowRightMenu()
{
	bRightMenuOpen = true;
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
	if (MyPC) {

	}
}

void ACodeAICharacter::HideRightMenu()
{
	bRightMenuOpen = false;
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
	if (MyPC) {

	}
}

void ACodeAICharacter::ActionPressed()
{
	//Use the item's ability
	if (bLeftMenuOpen) {
		InventoryArray[EquippedIndex]->MenuUse();
		AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
		if (MyPC) {
			MyPC->UpdateItem();
		}
	}
	//Knock on a wall
	else if (bIsInCover) {
		if (KnockingSound) {
			ReportNoise(KnockingSound, 1.5f);
		}
	}
}

void ACodeAICharacter::ActionReleased()
{

}

void ACodeAICharacter::UsePressed()
{
	bIsUsingItem = true;
	if (bItemEquipped) {
		InventoryArray[EquippedIndex]->UseItemPressed(true);
		if (!bUsingFPP)
		{
			ToogleCharacterControls(false);
		}
		AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
		if (MyPC && !bUsingFPP)
		{
			FInputModeGameAndUI NewInput;
			MyPC->SetInputMode(NewInput);
		}
	}
}

void ACodeAICharacter::UseReleased()
{
	bIsUsingItem = false;
	if ((bItemEquipped || !bAllowMovement)) {
		if (!bAllowMovement) {
			InventoryArray[EquippedIndex]->UseItemReleased(true);
			AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
			if (MyPC) {
				MyPC->UpdateItem();
			}
		}
		if (!bUsingFPP) {
			ToogleCharacterControls(true);
		}
	}
}

void ACodeAICharacter::ReloadPressed()
{
	AWeaponItem* Weapon = Cast<AWeaponItem>(InventoryArray[EquippedIndex]);
	if (Weapon) {
		Weapon->Reload();
		AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
		if (MyPC) {
			MyPC->UpdateItem();
		}
	}
}

void ACodeAICharacter::ReloadReleased()
{

}

void ACodeAICharacter::FPPPressed()
{
	if (!bIsDead) {
		bUsingFPP = true;

		FPPCamera->SetWorldRotation(GetActorRotation());
		bUseControllerRotationYaw = true;

		AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
		//If the player is inside a Camera Section, set the player as the view target
		if (SectionCamera)
		{
			if (MyPC)
			{
				MyPC->SetViewTarget(this);
			}
		}

		//Switch camera
		if (TopDownCamera->IsActive())
		{
			TopDownCamera->Deactivate();
		}
		else if (LeftCamera->IsActive())
		{
			LeftCamera->Deactivate();
		}
		else if (RightCamera->IsActive())
		{
			RightCamera->Deactivate();
		}
		FPPCamera->Activate();

		//Only toggle controls if character isn't already movement restricted
		if (bAllowMovement)
		{
			ToogleCharacterControls(false);
		}
		if (MyPC)
		{
			MyPC->bShowMouseCursor = false;
		}
	}
}

void ACodeAICharacter::FPPReleased()
{
	if (!bIsDead) {
		bUsingFPP = false;

		bUseControllerRotationYaw = false;

		AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
		//If the player is inside a Camera Section, set the SectionCamera as the view target
		if (SectionCamera)
		{
			if (MyPC)
			{
				MyPC->SetViewTarget(SectionCamera);
			}
		}

		FPPCamera->Deactivate();
		TopDownCamera->Activate();
		
		//If the player is already aiming, simply show mouse cursor
		if (!bIsUsingItem)
		{
			ToogleCharacterControls(true);
		}
		if (MyPC)
		{
			MyPC->bShowMouseCursor = true;
		}
	}
}

void ACodeAICharacter::PausePressed()
{
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
	if (MyPC) {
		MyPC->TooglePauseMenu();
	}
}

void ACodeAICharacter::CrouchPressed()
{
	if (bAllowMovement) {
		bIsCrouchPressed = true;

		bool bWasProne = bIsProne;
		if (!bIsProne) {
			bIsCrouching = !bIsCrouching;
		}
		else {
			PrepareCrouch();
		}
		//If the player holds the crouch button, enter prone
		if (bIsCrouching && !bWasProne)
		{
			GetWorld()->GetTimerManager().SetTimer(CrouchHandle, this, &ACodeAICharacter::PrepareProne, CrouchWait, false);
		}
	}
}

void ACodeAICharacter::CrouchReleased()
{
	bIsCrouchPressed = false;
	GetWorld()->GetTimerManager().ClearTimer(CrouchHandle);
	if (HorizontalCrouchMov != 0.f || VerticalCrouchMov != 0.f && !bIsProne) {
		GetWorld()->GetTimerManager().SetTimer(CrouchHandle, this, &ACodeAICharacter::PrepareProne, CrouchWait, false);
	}
}

void ACodeAICharacter::UpdateCrouchRotation(float NewHor, float NewVer)
{
	if (NewHor != 0.f || NewVer != 0.f) {
		FRotator NewRot = GetActorRotation();
		float newZ = 180 - 90 * NewHor;
		if (newZ > 180.f) {
			newZ += 45 * NewVer;
		}
		else if (newZ < 180.f) {
			newZ -= 45 * NewVer;
		}
		else {
			newZ = 90 - 90 * NewVer;
		}
		NewRot.Yaw = newZ;
		SetActorRotation(NewRot);
	}
}

void ACodeAICharacter::PrepareCrouch()
{
	bAllowMovement = false;
	
	if (bItemEquipped) {
		//Unequip the item if it is a weapon
		AWeaponItem* Weapon = Cast<AWeaponItem>(InventoryArray[EquippedIndex]);
		if (Weapon) {
			Weapon->CancelUse();
		}
	}
	/*
	if (CrouchMontage) {
		GetMesh()->PlayAnimation((UAnimationAsset*)CrouchMontage, false);
	}
	GetWorldTimerManager().SetTimer(AnimationHandle, this, &ACodeAICharacter::FinishProne, CrouchMontageLength);
	*/
	FinishProne();
}

void ACodeAICharacter::PrepareProne()
{
	bAllowMovement = false;
	if (bItemEquipped) {
		//Unequip the item if it is a weapon
		AWeaponItem* Weapon = Cast<AWeaponItem>(InventoryArray[EquippedIndex]);
		if (Weapon) {
			Weapon->CancelUse();
		}
	}
	/*
	if (ProneMontage) {
		GetMesh()->PlayAnimation((UAnimationAsset* )ProneMontage, false);
	}
	GetWorldTimerManager().SetTimer(AnimationHandle, this, &ACodeAICharacter::StartProne, ProneMontageLength);
	*/
	StartProne();
}

void ACodeAICharacter::StartProne()
{
	//GetMesh()->Stop();
	bIsProne = true;
	bAllowMovement = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	HorizontalCrouchMov = 0.f;
	VerticalCrouchMov = 0.f;
}

void ACodeAICharacter::FinishProne()
{
	//GetMesh()->Stop();
	bIsProne = false;
	bAllowMovement = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	HorizontalCrouchMov = 0.f;
	VerticalCrouchMov = 0.f;
}

void ACodeAICharacter::HandleStagger()
{
	if (bItemEquipped) {
		//Unequip the item if it is a weapon
		AWeaponItem* Weapon = Cast<AWeaponItem>(InventoryArray[EquippedIndex]);
		if (Weapon) {
			Weapon->CancelUse();
		}
	}
	bAllowMovement = false;
	if (StaggerAnimations.Num() > 0) {
		int Random = FMath::RandRange(0, StaggerAnimations.Num() - 1);
		//GetMesh()->PlayAnimation(StaggerAnimations[Random], false);
		//float Len = StaggerAnimations[Random];
	}
}

void ACodeAICharacter::FinishStagger()
{
	
}

bool ACodeAICharacter::AddItem(AGameItem * Item)
{
	bShouldAddItem = true;

	//Check if the item is ammo
	AAmmoItem* AmmoItem = Cast<AAmmoItem>(Item);
	if (AmmoItem){
		return AddAmmoItem(AmmoItem);
		AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
		if (MyPC) {
			MyPC->UpdateItem();
		}
	}
	//Check if the item is a weapon
	AWeaponItem* Weapon = Cast<AWeaponItem>(Item);
	if (Weapon) {
		return AddWeapon(Weapon);
	}
	//Check if the item is stackable
	AStackableGameItem* Stackable = Cast<AStackableGameItem>(Item);
	if (Stackable) {
		return AddStackable(Stackable);
	}
	//Check if the item is an inventory item
	AInventoryGameItem* InventoryItem = Cast<AInventoryGameItem>(Item);
	if (InventoryItem) {
		return AddNewItem(InventoryItem);
	}
	return false;
}

bool ACodeAICharacter::AddNewItem(AInventoryGameItem * NewItem)
{
	//Search all of the player's items for an item with the same name
	for (AInventoryGameItem* PlayerItem : InventoryArray) {
		if (PlayerItem->GetItemName() == NewItem->GetItemName()) {
			bShouldAddItem = false;
		}
	}
	if (bShouldAddItem) {
		InventoryArray.Add(NewItem);
		//NewItem->DestroyComponents();
		return true;
	}
	else {
		return false;
	}
}

bool ACodeAICharacter::AddAmmoItem(AAmmoItem * AmmoItem)
{
	//Search for a weapon that has compatible ammo
	for (AInventoryGameItem* PlayerItem : InventoryArray) {
		AWeaponItem* Weapon = Cast<AWeaponItem>(PlayerItem);

		if (Weapon && Weapon->GetItemName() == AmmoItem->GetWeaponName()) {
			//If the weapon has full ammo, the player can't pickup the AmmoItem
			if (Weapon->MagsAreFull()) {
				AmmoItem->UpdateText(false, true);
				return false;
			}

			else {
				Weapon->AddAmmo(AmmoItem->GetQuantity());
				AmmoItem->UpdateText(true, true);
				//AmmoItem->DestroyComponents();

				//Update weapon's ammo on the UI
				AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
				if (MyPC) {
					MyPC->UpdateItem();
				}
				return true;
			}
		}
	}
	AmmoItem->UpdateText(false, false);
	return false;
}

bool ACodeAICharacter::AddWeapon(AWeaponItem * WeaponItem)
{
	//Search all of the player's items
	for (AInventoryGameItem* PlayerItem : InventoryArray) {
		//If the player already has this WeaponItem, don't add it
		AWeaponItem* Item = Cast<AWeaponItem>(PlayerItem);
		if (Item && Item->GetItemName() == WeaponItem->GetItemName()) {
			bShouldAddItem = false;
			return false;
		}
	}
	if (bShouldAddItem) {
		InventoryArray.Add(WeaponItem);
		//WeaponItem->DestroyComponents();
		
		if (WeaponItem->IsPistol()) {
			WeaponItem->GetGunMesh()->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName("PistolGrip"));
		}
		else {
			WeaponItem->GetGunMesh()->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName("RifleGrip"));
		}
		WeaponItem->SetOwner(this);

		return true;
	}
	return false;
}

bool ACodeAICharacter::AddStackable(AStackableGameItem * StackableItem)
{
	for (AInventoryGameItem* PlayerItem : InventoryArray) {
		//If the player already has this item, simply increment the existing one
		AStackableGameItem* Item = Cast<AStackableGameItem>(PlayerItem);

		if (Item && Item->GetItemName() == StackableItem->GetItemName()) {
			bShouldAddItem = false;
			Item->AddQuantity(1);
			return true;
		}
	}
	if (bShouldAddItem) {
		InventoryArray.Add(StackableItem);
		//StackableItem->DestroyComponents();
		return true;
	}
	return false;
}

void ACodeAICharacter::EquipWeapon(class AWeaponItem* WeaponItem)
{
	WeaponItem->Equip();
	bPistolEquipped = WeaponItem->IsPistol() ? true : false;
	bRifleEquipped = !bPistolEquipped;
}

void ACodeAICharacter::SetEquippedIndex(int32 Index)
{
	if (PreviousIndex != Index) {

		if (PreviousIndex > 0)
		{
			InventoryArray[PreviousIndex]->Unequip();
		}

		if (Index < 1) {
			if (ItemUnequip)
			{
				UGameplayStatics::PlaySound2D(GetWorld(), ItemUnequip);
			}
		}
		else {
			if (ItemEquip)
			{
				UGameplayStatics::PlaySound2D(GetWorld(), ItemEquip);
			}
			if (!bItemEquipped) {
				bItemEquipped = true;
			}
			InventoryArray[Index]->Equip();
		}
	}
	else
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ItemEquip);
	}
}

void ACodeAICharacter::SetIndex(int32 Index)
{
	if (Index >= 0 && Index < InventoryArray.Num()) {
		EquippedIndex = Index;
	}
	else if (Index >= InventoryArray.Num()) {
		EquippedIndex = 0;
	}
	else {
		EquippedIndex = InventoryArray.Num() - 1;
	}
}

float ACodeAICharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f) {
		if (Health - DamageAmount > 0.f) {
			if (PlayerHurt_1 && PlayerHurt_2) {
				int i = FMath::RandRange(0, 1);
				if (i) {
					UGameplayStatics::PlaySound2D(GetWorld(), PlayerHurt_1);
				}
				else {
					UGameplayStatics::PlaySound2D(GetWorld(), PlayerHurt_2);
				}
			}
			Health -= DamageAmount;
			HUDHealth = Health / MaxHealth;
			//HandleStagger();
			return DamageAmount;
		}
		else {
			OnDeath();	
			Health = 0.f;
			HUDHealth = 0.f;
			return (DamageAmount);
		}
	}
	return 0.f;
}

void ACodeAICharacter::OnDeath()
{
	bIsDead = true;
	//Notify the enemies that the player was killed
	if (AttackingEnemies.Num() > 0) {
		for (AMyAICharacter* Enemy : AttackingEnemies) {
			if (!Enemy->IsDead()) {
				Enemy->PlayerKilled();
			}
		}
	}
	if (PlayerDead)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), PlayerDead);
	}

	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
	if (MyPC)
	{
		MyPC->ShowDeathMenu();
	}
	SwitchToDeathCamera();

	if (bItemEquipped)
	{
		//Unequip the item if it is a weapon
		AWeaponItem* Weapon = Cast<AWeaponItem>(InventoryArray[EquippedIndex]);
		if (Weapon)
		{
			Weapon->CancelUse();
		}
	}

	//Play random death animation
	if (!bIsProne)
	{
		if (DeathAnimations.Num() > 0)
		{
			int Random = FMath::RandRange(0, DeathAnimations.Num() - 1);
			GetMesh()->PlayAnimation(DeathAnimations[Random], false);
		}
	}
	else
	{
		if (ProneDeathAnimations.Num() > 0)
		{
			int Random = FMath::RandRange(0, ProneDeathAnimations.Num() - 1);
			GetMesh()->PlayAnimation(ProneDeathAnimations[Random], false);
		}
	}
}

void ACodeAICharacter::SwitchToDeathCamera()
{
	if (LeftCamera->IsActive()) {
		LeftCamera->Deactivate();
	}
	else if (RightCamera->IsActive()) {
		RightCamera->Deactivate();
	}
	else if (TopDownCamera->IsActive()) {
		TopDownCamera->Deactivate();
	}
	DeathBoom->AddWorldRotation(FRotator(FMath::RandRange(-30.f, 0.f), FMath::RandRange(0.f, 360.f), 0.f).Quaternion());
	DeathCamera->Activate();
}

void ACodeAICharacter::AddHealth(float Regen)
{
	if ((Health + Regen) <= MaxHealth) {
		Health += Regen;
		HUDHealth = Health / MaxHealth;
	}
	else {
		Health = MaxHealth;
		HUDHealth = 1.f;
	}
}

void ACodeAICharacter::AddEnemy(AMyAICharacter * NewEnemy)
{
	AttackingEnemies.AddUnique(NewEnemy);
}

void ACodeAICharacter::RemoveEnemy(AMyAICharacter * NewEnemy)
{
	AttackingEnemies.Remove(NewEnemy);
}

void ACodeAICharacter::ReportNoise(USoundBase* SoundToPlay, float Volume)
{
	//If we have a valid sound to play, play the sound and
	//report it to our game
	if (SoundToPlay)
	{
		//Play the actual sound
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, GetActorLocation(), 1.f);

		if (Volume < 4.f) {
			PawnNoiseEmitterComp->MakeNoise(this, Volume, GetActorLocation());
		}
		else {
			NotifyLoudNoise(GetActorLocation());
		}
	}

}

void ACodeAICharacter::MoveBlockedBy(const FHitResult & Impact)
{
	Super::MoveBlockedBy(Impact);

	if (!bIsInCover && !bIsProne && NoMov == 0.f) {

		AMGSCube* Cube = Cast<AMGSCube>(Impact.GetActor());
		if (Cube) {
			bool VecZero = GetCharacterMovement()->Velocity.IsNearlyZero();
			float Dot = FVector::DotProduct(GetActorForwardVector(), Impact.Normal);
			if (Dot <= -0.99 && !VecZero) {
				bIsInCover = true;
				if (PlayerCover) {
					UGameplayStatics::PlaySound2D(GetWorld(), PlayerCover);
				}
				FRotator Rot = Impact.Normal.Rotation();
				SetActorRotation(Rot.Quaternion());
				GetCharacterMovement()->bOrientRotationToMovement = false;
				GetCharacterMovement()->MaxWalkSpeed /= CoverSpeedDecrease;
			}
			else {
			}
		}
	}
}

void ACodeAICharacter::ToggleCamera(bool bDefaultCamera, bool bLeftCamera)
{
	if (!SectionCamera)
	{
		if (bDefaultCamera)
		{
			if (LeftCamera->IsActive())
			{
				LeftCamera->Deactivate();
			}
			else if (RightCamera->IsActive())
			{
				RightCamera->Deactivate();
			}
			TopDownCamera->Activate();
		}
		else
		{
			TopDownCamera->Deactivate();

			if (bLeftCamera)
			{
				LeftCamera->Activate();
			}
			else
			{
				RightCamera->Activate();
			}
		}
	}
}

void ACodeAICharacter::EnterCameraSection(ACameraActor * Camera)
{
	bIsInCameraSection = true;

	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
	if (MyPC)
	{
		MyPC->SetViewTarget(Camera);
	}
	SectionCamera = Camera;
}

void ACodeAICharacter::ExitCameraSection(ACameraActor * Camera)
{
	if (Camera == SectionCamera)
	{
		AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
		if (MyPC)
		{
			MyPC->SetViewTarget(this);
		}
		bIsInCameraSection = false;
		SectionCamera = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACodeAICharacter::SetupPlayerInputComponent(class UInputComponent* inputComponent)
{
	// Set up gameplay key bindings
	check(inputComponent);

	inputComponent->BindAxis("MoveForward", this, &ACodeAICharacter::MoveForward);
	inputComponent->BindAxis("MoveRight", this, &ACodeAICharacter::MoveRight).bExecuteWhenPaused = true;

	inputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	inputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	inputComponent->BindAction("Walk", IE_Pressed, this, &ACodeAICharacter::Walk);
	inputComponent->BindAction("Walk", IE_Released, this, &ACodeAICharacter::StopWalking);

	inputComponent->BindAction("LeftMenu", IE_Pressed, this, &ACodeAICharacter::PressedLeftMenu);
	inputComponent->BindAction("LeftMenu", IE_Released, this, &ACodeAICharacter::HideLeftMenu).bExecuteWhenPaused = true;

	inputComponent->BindAction("RightMenu", IE_Pressed, this, &ACodeAICharacter::ShowRightMenu);
	inputComponent->BindAction("RightMenu", IE_Released, this, &ACodeAICharacter::HideRightMenu).bExecuteWhenPaused = true;

	inputComponent->BindAction("Action", IE_Pressed, this, &ACodeAICharacter::ActionPressed).bExecuteWhenPaused = true;;
	inputComponent->BindAction("Action", IE_Released, this, &ACodeAICharacter::ActionReleased);

	inputComponent->BindAction("Use", IE_Pressed, this, &ACodeAICharacter::UsePressed);
	inputComponent->BindAction("Use", IE_Released, this, &ACodeAICharacter::UseReleased);

	inputComponent->BindAction("Reload", IE_Pressed, this, &ACodeAICharacter::ReloadPressed);
	inputComponent->BindAction("Reload", IE_Released, this, &ACodeAICharacter::ReloadReleased);

	inputComponent->BindAction("FPP", IE_Pressed, this, &ACodeAICharacter::FPPPressed);
	inputComponent->BindAction("FPP", IE_Released, this, &ACodeAICharacter::FPPReleased);

	inputComponent->BindAction("Crouch", IE_Pressed, this, &ACodeAICharacter::CrouchPressed);
	inputComponent->BindAction("Crouch", IE_Released, this, &ACodeAICharacter::CrouchReleased);

	inputComponent->BindAction("Pause", IE_Pressed, this, &ACodeAICharacter::PausePressed).bExecuteWhenPaused = true;

	//inputComponent->BindAction("Crouch", IE_Pressed, this, &ACodeAICharacter::FPPPressed);
	//inputComponent->BindAction("Crouch", IE_Released, this, &ACodeAICharacter::FPPReleased);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	
	
	inputComponent->BindAxis("Turn", this, &ACodeAICharacter::MouseTurnX);
	//inputComponent->BindAxis("TurnRate", this, &ACodeAICharacter::TurnAtRate);
	inputComponent->BindAxis("LookUp", this, &ACodeAICharacter::MouseTurnY);
	//inputComponent->BindAxis("LookUpRate", this, &ACodeAICharacter::LookUpAtRate);
	
}

void ACodeAICharacter::Walk()
{
	bIsWalking = true;

	GetCharacterMovement()->MaxWalkSpeed /= WalkSpeedDecrease;
}

void ACodeAICharacter::StopWalking()
{
	bIsWalking = false;

	GetCharacterMovement()->MaxWalkSpeed *= WalkSpeedDecrease;
}

void ACodeAICharacter::NotifyLoudNoise(FVector Loc)
{
	if (AllEnemies.Num() > 0) {
		for (AMyAICharacter* Enemy : AllEnemies) {
			if (!Enemy->IsDead()) {
				Enemy->RunToHeardSound(Loc);
			}
		}
	}
}

bool ACodeAICharacter::CoverLineTrace(bool bLeft, bool bForward)
{
	FVector Start = GetActorLocation();
	Start.Z += 30.f;
	//Changes the offset depending on whether the trace is supposed
	//to be executed to the left or right
	float Offset = bLeft ? CoverPeakDistance : -CoverPeakDistance;
	//Changes the vector's direction,
	//depending on the orientation of cover
	Start += bForward ? FVector(0.f, Offset, 0.f) : FVector(Offset, 0.f, 0.f);
	FVector End = Start - (GetActorForwardVector() * 100);
	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility)) {
		return Hit.bBlockingHit;
	}
	else {
		return false;
	}
}

void ACodeAICharacter::TurnAtRate(float Rate)
{

}

void ACodeAICharacter::LookUpAtRate(float Rate)
{

}

void ACodeAICharacter::MouseTurnX(float Rate)
{
	XRate = Rate;
}

void ACodeAICharacter::MouseTurnY(float Rate)
{
	YRate = Rate;
}

void ACodeAICharacter::MoveForward(float Value)
{
	if (bAllowMovement) {

		if (!bIsProne) {
			if (Controller != NULL && !bIsCrouching) {

				//Reset CrouchMov
				if (VerticalCrouchMov != 0.f) {
					VerticalCrouchMov = 0.f;
				}

				//If the value is not 0, and the player is either not in cover or in cover
				//that was triggered from the other movement direction
				if (Value != 0.0f && (!bIsInCover || (bShouldBeInCoverRight && (Value != NoMov || !bNoMovForward)))) {

					// get forward vector
					const FVector Direction = FRotationMatrix(FRotator(0.f, -1.f, 0.f)).GetUnitAxis(EAxis::X);
					AddMovementInput(Direction, Value);

					//Reset "illegal" movement
					if (NoMov != 0.f) {
						NoMov = 0.f;
					}
				}

				if (Value == 0.f && !TopDownCamera->IsActive()) {
					ToggleCamera(true);
					if (NoMov != 0.f) {
						NoMov = 0.f;
					}
				}
				ForwardMov = Value;
			}

			//If the player is crouching
			else if (Controller != NULL) {
				HandleVerticalCrouch(Value);
			}
		}
		//If the player is prone
		else {
			HandleProneMovement(Value);
		}
	}
}

void ACodeAICharacter::HandleVerticalCrouch(float Value)
{
	if (Value != VerticalCrouchMov) {
		//If the registered value changes, clear the timer
		GetWorld()->GetTimerManager().ClearTimer(CrouchHandle);
		UpdateCrouchRotation(HorizontalCrouchMov, VerticalCrouchMov);
		if (!(Value == 0.f && VerticalCrouchMov == 0.f)) {
			//If the player tries to move for a certain amount of time, trigger prone
			GetWorld()->GetTimerManager().SetTimer(CrouchHandle, this, &ACodeAICharacter::PrepareProne, CrouchWait, false);
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(CrouchHandle);
		}
	}
	VerticalCrouchMov = Value;
}

void ACodeAICharacter::HandleProneMovement(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value * ProneMovement);
}

void ACodeAICharacter::MoveRight(float Value)
{
	if (bAllowMovement) {
		if (!bLeftMenuOpen) {

			if (!bIsProne) {
				if (Controller != NULL && !bIsCrouching) {
					if (HorizontalCrouchMov != 0.f) {
						HorizontalCrouchMov = 0.f;
					}
					//If the value is not 0, and the player is either not in cover or in cover
					//that was triggered from the other movement direction
					if (Value != 0.0f && (!bIsInCover || (bShouldBeInCoverForward && (Value != NoMov || bNoMovForward)))) {

						// get right vector 
						const FVector Direction = FRotationMatrix(FRotator(0.f, -1.f, 0.f)).GetUnitAxis(EAxis::Y);
						// add movement in that direction
						AddMovementInput(Direction, Value);

						//Reset "illegal" movement
						if (NoMov != 0.f) {
							NoMov = 0.f;
						}
					}

					if (Value == 0.f && !TopDownCamera->IsActive()) {
						ToggleCamera(true);
						if (NoMov != 0.f) {
							NoMov = 0.f;
						}
					}
					RightMov = Value;
				}

				//If the player is crouching
				else if (bIsCrouching) {
					HandleHorizontalCrouch(Value);
				}
			}
			//If the player is prone
			else {
				HandleProneRotation(Value);
			}
		}
		//If the player is using the menu
		else {
			HandleMenuInput(Value);
		}
	}
}

void ACodeAICharacter::HandleHorizontalCrouch(float Value)
{
	if (Value != HorizontalCrouchMov) {
		//If the registered value changes, clear the timer
		GetWorld()->GetTimerManager().ClearTimer(CrouchHandle);
		UpdateCrouchRotation(HorizontalCrouchMov, VerticalCrouchMov);
		if (!(Value == 0.f && VerticalCrouchMov == 0.f)) {
			//If the player tries to move for a certain amount of time, trigger prone
			GetWorld()->GetTimerManager().SetTimer(CrouchHandle, this, &ACodeAICharacter::PrepareProne, CrouchWait, false);
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(CrouchHandle);
		}
	}
	HorizontalCrouchMov = Value;
}

void ACodeAICharacter::HandleProneRotation(float Value)
{
	SetActorRotation(GetActorRotation() + FRotator(0.f, Value, 0.f));
}

void ACodeAICharacter::HandleMenuInput(float Value)
{
	if (bAllowNavigation) {
		AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());

		if (Value > 0.f && MyPC) {
			bAllowNavigation = false;
			MyPC->MenuRight();
			SetIndex(EquippedIndex - 1);
			if (MenuSound) {
				UGameplayStatics::PlaySound2D(GetWorld(), MenuSound);
			}
		}
		else if (Value < 0.f && MyPC) {
			bAllowNavigation = false;
			MyPC->MenuLeft();
			SetIndex(EquippedIndex + 1);
			if (MenuSound) {
				UGameplayStatics::PlaySound2D(GetWorld(), MenuSound);
			}
		}
	}
	else if (!bAllowNavigation && Value == 0.f) {
		bAllowNavigation = true;
	}
}

void ACodeAICharacter::ToogleCharacterControls(bool bAllow)
{
	bAllowMovement = bAllow;

	//Movement controls
	if (bAllowMovement) {
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
	//Firing controls
	else {
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}



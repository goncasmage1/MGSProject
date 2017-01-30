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
	bIsCrouching = false;
	bIsCrouchPressed = false;
	bIsProne = false;
	bIsStaggering = false;
	bIsDead = false;

	EquippedIndex = ExtraIndex = 0;
	PreviousIndex = -1;

	WalkSpeedDecrease = 3.f;
	CoverSpeedDecrease = 2.5f;
	ForwardMov = RightMov = NoMov = 0.f;
	HorizontalCrouchMov = VerticalCrouchMov = 0.f;
	XRate = YRate = 0.f;
	MenuTimer = 0.f;
	MenuHeldDownTime = 0.2f;
	CrouchWait = 0.2f;
	ProneMovement = 0.5f;
	CoverPeakDistance = 60.f;

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
}

void ACodeAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsInCover) {
		HandleCoverLogic();
		HandleCoverLineTrace();
	}
	if (!bLeftMenuOpen && bLeftMenuPressed) {
		MenuTimer += DeltaTime;
		if (MenuTimer >= MenuHeldDownTime) {
			ShowLeftMenu();
			MenuTimer = 0.f;
		}
	}
	/*
	if (!bAllowMovement && !bUsingFPP) {
		HandlePlayerRotation();
	}
	*/
}

void ACodeAICharacter::HandlePlayerRotation()
{
	if (YRate != 0.f && XRate != 0.f) {
		SetActorRotation(GetControlRotation());
	}
}

void ACodeAICharacter::HandleCoverLogic()
{
	//When the player enters cover from a forward position
	if (ForwardMov != 0.0f && !bShouldBeInCoverForward && !bShouldBeInCoverRight && NoMov == 0) {
		bShouldBeInCoverForward = true;
		return;
	}
	//When the player enters cover from a right position
	else if (RightMov != 0.0f && !bShouldBeInCoverForward && !bShouldBeInCoverRight && NoMov == 0) {
		bShouldBeInCoverRight = true;
		return;
	}
	//When the player exits cover from a forward position
	else if (ForwardMov == 0.0f && bShouldBeInCoverForward) {
		OnEndCover();
		return;
	}
	//When the player exits cover from a right position
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
	if ((ForwardMov > 0.f && RightMov > 0.f) || (ForwardMov < 0.f && RightMov > 0.f)) {
		if (!InCoverLeftLineTrace(true, true)) {
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
	else if ((ForwardMov > 0.f && RightMov < 0.f) || (ForwardMov < 0.f && RightMov < 0.f)) {
		if (!InCoverLeftLineTrace(false, true)) {
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
	if ((RightMov > 0.f && ForwardMov > 0.f) || (RightMov < 0.f && ForwardMov > 0.f)) {
		if (!InCoverLeftLineTrace(true, false)) {
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
	else if ((RightMov > 0.f && ForwardMov < 0.f) || (RightMov < 0.f && ForwardMov < 0.f)) {
		if (!InCoverLeftLineTrace(false, false)) {
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
	MenuTimer = 0.f;
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
	if (bLeftMenuOpen) {
		bLeftMenuOpen = false;
		if (MyPC) {
			MyPC->ToogleLeftMenu();
		}
	}
	else {
		if (bItemEquipped) {
			bItemEquipped = false;
			//Play sound
			if (ItemEquip) {
				UGameplayStatics::PlaySound2D(GetWorld(), ItemUnequip);
			}
			//Unequip the item if it is a weapon
			AWeaponItem* Weapon = Cast<AWeaponItem>(InventoryArray[EquippedIndex]);
			if (Weapon) {
				Weapon->Unequip();
			}
		}
		else {
			bItemEquipped = true;
			//Play sound
			if (ItemUnequip) {
				UGameplayStatics::PlaySound2D(GetWorld(), ItemEquip);
			}
			AWeaponItem* Weapon = Cast<AWeaponItem>(InventoryArray[EquippedIndex]);
			if (Weapon) {
				Weapon->Equip();
			}
		}
		if (MyPC) {
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
	if (bLeftMenuOpen) {
		InventoryArray[EquippedIndex]->MenuUse();
		AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
		if (MyPC) {
			MyPC->UpdateItem();
		}
	}
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
	if (bItemEquipped) {
		InventoryArray[EquippedIndex]->UseItemPressed(true);
		ToogleCharacterControls(false);
	}
}

void ACodeAICharacter::UseReleased()
{
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
		if (TopDownCamera->IsActive()) {
			TopDownCamera->Deactivate();
		}
		else if (LeftCamera->IsActive()) {
			LeftCamera->Deactivate();
		}
		else if (RightCamera->IsActive()) {
			RightCamera->Deactivate();
		}
		FPPCamera->Activate();
		ToogleCharacterControls(false);
	}
}

void ACodeAICharacter::FPPReleased()
{
	if (!bIsDead) {
		bUsingFPP = false;
		FPPCamera->Deactivate();
		TopDownCamera->Activate();
		ToogleCharacterControls(true);
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
		if (!bIsProne) {
			bIsCrouching = !bIsCrouching;
		}
		else {
			PrepareCrouch();
		}
		GetWorld()->GetTimerManager().SetTimer(CrouchHandle, this, &ACodeAICharacter::PrepareProne, CrouchWait, false);
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

void ACodeAICharacter::UpdateRotation(float NewHor, float NewVer)
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

void ACodeAICharacter::PrepareProne()
{
	bAllowMovement = false;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Prone"));
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
	GetMesh()->Stop();
	bIsProne = true;
	bAllowMovement = true;
	HorizontalCrouchMov = 0.f;
	VerticalCrouchMov = 0.f;
}

void ACodeAICharacter::PrepareCrouch()
{
	bAllowMovement = false;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Unprone"));
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

void ACodeAICharacter::FinishProne()
{
	GetMesh()->Stop();
	bIsProne = false;
	bAllowMovement = true;
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
	//Search all of the player's items
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
	for (AInventoryGameItem* PlayerItem : InventoryArray) {
		//If the item is ammo
		AWeaponItem* Weapon = Cast<AWeaponItem>(PlayerItem);
		if (Weapon && Weapon->GetItemName() == AmmoItem->GetWeaponName()) {
			if (Weapon->MagsAreFull()) {
				AmmoItem->UpdateText(true);
				return false;
			}
			else {
				Weapon->AddAmmo(AmmoItem->GetQuantity());
				AmmoItem->UpdateText(false);
				//AmmoItem->DestroyComponents();
				AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
				if (MyPC) {
					MyPC->UpdateItem();
				}
				return true;
			}
		}
	}
	return false;
}

bool ACodeAICharacter::AddWeapon(AWeaponItem * WeaponItem)
{
	//Search all of the player's items
	for (AInventoryGameItem* PlayerItem : InventoryArray) {
		//If the items have the same name
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
		//If the items have the same name
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
	bRifleEquipped = WeaponItem->IsPistol() ? false : true;
}

void ACodeAICharacter::SetEquippedIndex(int32 Index)
{
	if (PreviousIndex != Index) {
		//If the previously equipped item was a weapon, unequip it
		AWeaponItem* PreviousWeapon = Cast<AWeaponItem>(InventoryArray[PreviousIndex]);
		if (PreviousWeapon) {
			PreviousWeapon->Unequip();
		}

		//If the newly equipped item is a weapon, equip it
		AWeaponItem* NewWeapon = Cast<AWeaponItem>(InventoryArray[Index]);
		if (NewWeapon) {
			NewWeapon->Equip();
		}

		if (Index == 0) {
			UGameplayStatics::PlaySound2D(GetWorld(), ItemUnequip);
		}
		else {
			UGameplayStatics::PlaySound2D(GetWorld(), ItemEquip);
			if (!bItemEquipped) {
				bItemEquipped = true;
			}
		}
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
	if (AttackingEnemies.Num() > 0) {
		for (AMyAICharacter* Enemy : AttackingEnemies) {
			if (!Enemy->IsDead()) {
				Enemy->PlayerKilled();
			}
		}
	}
	if (PlayerDead) {
		UGameplayStatics::PlaySound2D(GetWorld(), PlayerDead);
	}
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetController());
	if (MyPC) {
		MyPC->ShowDeathMenu();
	}
	SwitchToDeathCamera();

	if (bItemEquipped) {
		//Unequip the item if it is a weapon
		AWeaponItem* Weapon = Cast<AWeaponItem>(InventoryArray[EquippedIndex]);
		if (Weapon) {
			Weapon->CancelUse();
		}
	}

	if (DeathAnimations.Num() > 0) {
		int Random = FMath::RandRange(0, DeathAnimations.Num() - 1);
		GetMesh()->PlayAnimation(DeathAnimations[Random], false);
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

	if (!bIsInCover && NoMov == 0.f) {

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
	if (bDefaultCamera) {
		if (LeftCamera->IsActive()) {
			LeftCamera->Deactivate();
		}
		else if (RightCamera->IsActive()) {
			RightCamera->Deactivate();
		}
		TopDownCamera->Activate();
	}
	else {
		TopDownCamera->Deactivate();

		if (bLeftCamera) {
			LeftCamera->Activate();
		}
		else {
			RightCamera->Activate();
		}
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
	
	
	inputComponent->BindAxis("Turn", this, &ACodeAICharacter::AddControllerYawInput);
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

bool ACodeAICharacter::InCoverLeftLineTrace(bool bToTheLeft, bool bForward)
{
	FVector Start = GetActorLocation();
	Start.Z += 30.f;
	//Changes the offset depending on whether the trace is supposed
	//to be executed to the left or right
	float Offset;
	if (bToTheLeft) {
		Offset = CoverPeakDistance;
	}
	else {
		Offset = -CoverPeakDistance;
	}
	//Changes the vector to add to the start vector,
	//depending on the orientation of the cover
	if (bForward) {
		Start += FVector(0.f, Offset, 0.f);
	}
	else {
		Start += FVector(Offset, 0.f, 0.f);
	}

	FVector End = Start - (GetActorForwardVector() * 100);
	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility)) {
		//DrawDebugLine(GetWorld(), Start, End, FColor::Green, true);
		if (Hit.bBlockingHit) {
			return true;
		}
		else {
			return false;
		}
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
			else if (bIsCrouching) {
				HandleVerticalCrouch(Value);
			}
		}
		else {
			HandleProneMovement(Value);
		}
	}
}

void ACodeAICharacter::HandleVerticalCrouch(float Value)
{
	if (Value != VerticalCrouchMov) {
		GetWorld()->GetTimerManager().ClearTimer(CrouchHandle);
		UpdateRotation(HorizontalCrouchMov, VerticalCrouchMov);
		if (!(Value == 0.f && VerticalCrouchMov == 0.f)) {
			GetWorld()->GetTimerManager().SetTimer(CrouchHandle, this, &ACodeAICharacter::PrepareProne, CrouchWait, false);
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
		else {
			HandleProneRotation(Value);
		}
	}
	else {
		if (bLeftMenuOpen) {
			HandleMenuInput(Value);
		}
	}
}

void ACodeAICharacter::HandleHorizontalCrouch(float Value)
{
	if (Value != HorizontalCrouchMov) {
		GetWorld()->GetTimerManager().ClearTimer(CrouchHandle);
		UpdateRotation(HorizontalCrouchMov, VerticalCrouchMov);
		if (!(Value == 0.f && VerticalCrouchMov == 0.f)) {
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("No Timer"));
			GetWorld()->GetTimerManager().SetTimer(CrouchHandle, this, &ACodeAICharacter::PrepareProne, CrouchWait, false);
		}
	}
	HorizontalCrouchMov = Value;
}

void ACodeAICharacter::HandleProneRotation(float Value)
{
	GEngine->AddOnScreenDebugMessage(-1, .2f, FColor::Red, TEXT("Rotating"));
	SetActorRotation(GetActorRotation() + FRotator(0.f, 2.5f * Value, 0.f));
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

	if (bAllowMovement) {
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
	else {
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
		Controller->SetControlRotation(GetActorRotation());
	}
}


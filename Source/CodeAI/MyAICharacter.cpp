// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "MyAICharacter.h"
#include "AnAIController.h"
#include "CodeAICharacter.h"
#include "PlayerHUD.h"
#include "MyPlayerController.h"
#include "BotTargetPoint.h"
#include "WeaponItem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/PawnSensingComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AMyAICharacter::AMyAICharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Initializing the pawn sensing component
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	//Initialize the Pawn Noise Emitter
	PawnNoiseEmitterComp = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("PawnNoiseEmitterComp"));

	MinHearingThresh = 100.f;

	ChasingPawn = nullptr;

	TargetPointNum = 0;

	DealingDamage = 20.f;
	MaxHealth = 100.f;
	Health = 100.f;
	MinAttackRate = 1.f;
	MaxAttackRate = 1.5f;
	ExtraTime = FMath::RandRange(MinAttackRate, MinAttackRate + MaxAttackRate);
	Time = ExtraTime / 2;

	bCanSeePlayer = false;
	bHeardNoise = false;
	bHeldUp = false;
	bSightBlockedByCover = false;
	bReverseTargetPoints = false;
	bIsAiming = false;
}

// Called when the game starts or when spawned
void AMyAICharacter::BeginPlay()
{
	Super::BeginPlay();

	//Register the function that is going to fire when the character sees a Pawn
	if (PawnSensingComp) {
		//PawnSensingComp->OnSeePawn.AddDynamic(this, &AMyAICharacter::OnSeePlayer);
		PawnSensingComp->OnHearNoise.AddDynamic(this, &AMyAICharacter::OnHearNoise);
	}
	if (Weapon) {
		Weapon->AIEquip(this);
		if (Weapon->IsPistol()) {
			bIsPistolEquipped = true;
			Weapon->GetGunMesh()->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName("PistolGrip"));
		}
		else {
			bIsRifleEquipped = true;
			Weapon->GetGunMesh()->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName("RifleGrip"));
		}
	}

	//Get a reference to the player character
	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACodeAICharacter::StaticClass(), Characters);
	for (AActor* Char : Characters) {
		ACodeAICharacter* NewChar = Cast<ACodeAICharacter>(Char);
		if (NewChar) {
			ChasingPawn = NewChar;
		}
	}
}

void AMyAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If the pawn can sense anything, stop chasing the player
	if (bCanSeePlayer) {
		Time += DeltaTime;
		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ChasingPawn->GetActorLocation()));
		if (Time >= ExtraTime) {
			Time = 0.f;
			ExtraTime = FMath::RandRange(MinAttackRate, MaxAttackRate);
			ShootWeapon();
		}
		if (!CheckLineOfSightTo(ChasingPawn)) {

			AAnAIController* AIController = Cast<AAnAIController>(GetController());
			if (AIController) {
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("No see"));
				AIController->SetPlayerLocation(ChasingPawn->GetActorLocation());
				AIController->SetState(EAIState::AI_ChasingTarget);
				AIController->ClearSeenTarget();
				LowerWeapon();
				bCanSeePlayer = false;
				bSightBlockedByCover = false;
				Time = 0.f;
				ExtraTime = FMath::RandRange(MinAttackRate, MinAttackRate + MaxAttackRate);
			}
		}
	}
	//If the player is in sight but is also in cover at an inapropriate angle
	else if (bSightBlockedByCover && !ChasingPawn->InCover() && CheckLineOfSightTo(ChasingPawn)) {
		AAnAIController* AIController = Cast<AAnAIController>(GetController());
		if (AIController) {
			DetectPlayer(AIController, ChasingPawn);
		}
	}
	//If the player is within sight
	else if (CheckLineOfSightTo(ChasingPawn)){
		PlayerSeen();
	}
	
}

void AMyAICharacter::OnSeePlayer(APawn* Pawn)
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	//Set the seen target on the blackboard
	if (AIController) {
		AMyAICharacter* AIChar = Cast<AMyAICharacter>(Pawn);
		if (AIChar) {
			if (AIChar->IsHeldUp()) {
				AIChar->FreeFromHoldUp();
			}
		}
	}
}

void AMyAICharacter::PlayerSeen()
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	//Set the seen target on the blackboard
	if (AIController) {
		//If the AI is already following the player, ignore this function
		if (AIController->GetSeenTarget()) {
			return;
		}

		if (ChasingPawn) {
			if (ChasingPawn->InCover()) {
				float Dot = FVector::DotProduct(ChasingPawn->GetActorForwardVector(), GetActorForwardVector());

				if (Dot <= 0.1) {
					DetectPlayer(AIController, ChasingPawn);
				}
				else {
					bSightBlockedByCover = true;
				}
			}
			else {
				DetectPlayer(AIController, ChasingPawn);
			}
		}
	}
}

void AMyAICharacter::OnHearNoise(APawn * PawnInstigator, const FVector & Location, float Volume)
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController && (AIController->GetState() != EAIState::AI_HeldUp)) {
		
		AMyAICharacter* AICharacter = Cast<AMyAICharacter>(PawnInstigator);
		ACodeAICharacter* Player = Cast<ACodeAICharacter>(PawnInstigator);
		//If the noise came from a friendly AI Character
		if (AICharacter) {
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("I'll help"));
			AIController->SetHelpBot(Cast<APawn>(AICharacter));
			AIController->SetState(EAIState::AI_HelpingFriendly);
			AIController->SetSlowSpeed(false);
		}
		//If the noise came from the player
		else if (!bCanSeePlayer && Player){
			FVector Len = GetActorLocation() - Location;
			//If the player is too close for comfort
			if (Len.Size() <= MinHearingThresh && !Player->InCover()) {
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Too close buddy"));
				DetectPlayer(AIController, PawnInstigator);
			}
			else {
				AIController->SetHeardLocation(Location);
				AIController->SetState(EAIState::AI_ChasingSound);
				AIController->SetSlowSpeed(true);
				//If there is a sound, play it
				if (SoundHeardSound && !bHeardNoise) {
					ReportNoise(SoundHeardSound, 1.f);
				}
				bHeardNoise = true;
			}
		}
	}
}

void AMyAICharacter::DetectPlayer(class AAnAIController* AICon, APawn * Pawn)
{
	if (AICon->GetState() != EAIState::AI_HeldUp) {
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("I see you"));
		//If there is a sound and the AI isn't already chasing the player, play it
		if (PlayerFoundSound && AICon->GetState() != EAIState::AI_ChasingTarget) {
			ReportNoise(PlayerFoundSound, 1.f);
		}
		AICon->SetSeenTarget(Pawn);
		AICon->SetState(EAIState::AI_AttackingTarget);
		AICon->SetSlowSpeed(false);
		bCanSeePlayer = true;
		AimWeapon();
	}
}

// Called to bind functionality to input
void AMyAICharacter::SetupPlayerInputComponent(class UInputComponent* inputComponent)
{
	Super::SetupPlayerInputComponent(inputComponent);

}

void AMyAICharacter::ReportNoise(USoundBase * SoundToPlay, float Volume)
{
	if (SoundToPlay)
	{
		//Play the actual sound
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, GetActorLocation(), Volume);

		//Report that we've played a sound with a certain volume in a specific location
		PawnNoiseEmitterComp->MakeNoise(this, Volume, GetActorLocation());
	}
}

bool AMyAICharacter::AddWeapon(AWeaponItem * WeaponItem)
{
	return false;
}

void AMyAICharacter::AimWeapon()
{
	if (Weapon) {
		bIsAiming = true;
		Weapon->UseItemPressed();
	}
}

void AMyAICharacter::LowerWeapon()
{
	if (Weapon) {
		bIsAiming = false;
		Weapon->CancelUse();
	}
}

void AMyAICharacter::ShootWeapon()
{
	if (Weapon) {
		Weapon->UseItemReleased();
	}
}

void AMyAICharacter::SufferHoldUp()
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController &&
			AIController->GetState() != EAIState::AI_HeldUp &&
			AIController->GetState() != EAIState::AI_AttackingTarget &&
			AIController->GetState() != EAIState::AI_ChasingTarget) {
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("HeldUp"));
			AIController->SetState(EAIState::AI_HeldUp);
			bHeldUp = true;
		if (HeldUpSound) {
			UGameplayStatics::PlaySound2D(GetWorld(), HeldUpSound);
		}
	}
}

bool AMyAICharacter::CheckLineOfSightTo(APawn * pawn)
{
	if (PawnSensingComp->HasLineOfSightTo(pawn)) {
		FVector LineAux = GetActorLocation() - pawn->GetActorLocation();
		//Divides each float of the line vector by the size of the vector to get a size of 1
		FVector Line = FVector((LineAux.X / LineAux.Size()), (LineAux.Y / LineAux.Size()), (LineAux.Z / LineAux.Size()));

		float Distance = LineAux.Size();
		float MaxDistance = (PawnSensingComp->SightRadius / 2) / UKismetMathLibrary::Tan(FMath::DegreesToRadians(PawnSensingComp->GetPeripheralVisionAngle()));
		float Angle = FVector::DotProduct(GetActorForwardVector(), Line);

		if (Distance <= MaxDistance &&
				Angle < - (1.f - PawnSensingComp->GetPeripheralVisionAngle() / 90.f)) {
			return true;
		}
		return false;
	}
	return false;
}

void AMyAICharacter::FreeFromHoldUp()
{
	bHeldUp = false;
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController) {
		AIController->SetState(EAIState::AI_Patrolling);
	}
}

void AMyAICharacter::IncrementTargetNum()
{
	if (!bResetTargetPoints) {
		TargetPointNum = bReverseTargetPoints ? TargetPointNum - 1 : TargetPointNum + 1;
		if (TargetPointNum == TargetPoints.Num() - 1) {
			bReverseTargetPoints = true;
		}
		else if (TargetPointNum == 0) {
			bReverseTargetPoints = false;
		}
	}
	else {
		TargetPointNum++;
		if (TargetPointNum > TargetPoints.Num() - 1) {
			TargetPointNum = 0;
		}
	}
}

void AMyAICharacter::SetNoiseHeard(bool bHeard)
{
	bHeardNoise = bHeard;
}

float AMyAICharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (!bCanSeePlayer) {
		AMyPlayerController* PC = Cast<AMyPlayerController>(EventInstigator);
		AAnAIController* AIController = Cast<AAnAIController>(GetController());
		if (PC && AIController) {
			AIController->SetPlayerLocation(PC->GetPawn()->GetActorLocation());
			AIController->SetState(EAIState::AI_ChasingTarget);
			AIController->SetSlowSpeed(false);
			bHeldUp = false;
			if (PlayerFoundSound) {
				ReportNoise(PlayerFoundSound, 1.f);
			}
		}
	}
	if (ActualDamage > 0.f) {
		if (Health - DamageAmount > 0.f) {
			if (DamagedSound) {
				ReportNoise(DamagedSound, 1.f);
			}
			Health -= DamageAmount;
			return DamageAmount;
		}
		else {
			if (KilledSound) {
				ReportNoise(KilledSound, 1.f);
			}
			Health = 0.f;
			if (Weapon) {
				Weapon->Destroy();
			}
			APlayerHUD* HUD = Cast<APlayerHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
			if (HUD) {
				HUD->RemoveFromRadar(this);
			}
			Destroy();
			return (DamageAmount - Health);
		}
	}
	return 0.f;
}
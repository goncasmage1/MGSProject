// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "MyAICharacter.h"
#include "AnAIController.h"
#include "CodeAICharacter.h"
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
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AMyAICharacter::OnSeePlayer);
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
				ChasingPawn = nullptr;
				bCanSeePlayer = false;
				bSightBlockedByCover = false;
				Time = 0.f;
				ExtraTime = FMath::RandRange(MinAttackRate, MinAttackRate + MaxAttackRate);
			}
		}
	}
	else if (bSightBlockedByCover && !ChasingPawn->InCover() && CheckLineOfSightTo(ChasingPawn)) {
		AAnAIController* AIController = Cast<AAnAIController>(GetController());
		if (AIController) {
			DetectPlayer(AIController, ChasingPawn);
		}
	}
}

void AMyAICharacter::OnSeePlayer(APawn* Pawn)
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	//Set the seen target on the blackboard
	if (AIController) {
		//If the AI is already following the player, ignore this function
		if (AIController->GetSeenTarget()) {
			return;
		}
		ChasingPawn = Cast<ACodeAICharacter>(Pawn);

		if (ChasingPawn->InCover()) {
			float Dot = FVector::DotProduct(ChasingPawn->GetActorForwardVector(), GetActorForwardVector());

			if (Dot <= 0.1) {
				DetectPlayer(AIController, Pawn);
			}
			else {
				bSightBlockedByCover = true;
			}
		}
		else {
			DetectPlayer(AIController, Pawn);
		}
	}
}

void AMyAICharacter::OnHearNoise(APawn * PawnInstigator, const FVector & Location, float Volume)
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController) {
		
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
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("I see you"));
	AICon->SetSeenTarget(Pawn);
	AICon->SetState(EAIState::AI_AttackingTarget);
	AICon->SetSlowSpeed(false);
	if (ChasingPawn == nullptr) {
		ChasingPawn = Cast<ACodeAICharacter>(Pawn);
	}
	bCanSeePlayer = true;
	//If there is a sound, play it
	if (PlayerFoundSound) {
		ReportNoise(PlayerFoundSound, 1.f);
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
		Weapon->UseItemPressed(this);
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
		Weapon->UseItemReleased(this);
	}
}

void AMyAICharacter::SufferHoldUp()
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController) {
		if (!(AIController->GetState() == EAIState::AI_AttackingTarget) &&
			!(AIController->GetState() == EAIState::AI_ChasingTarget && GetVelocity().Size() != 0.f))
		AIController->SetState(EAIState::AI_HeldUp);
	}
}

bool AMyAICharacter::CheckLineOfSightTo(APawn * pawn)
{
	if (PawnSensingComp->HasLineOfSightTo(pawn)) {
		FVector Line = GetActorLocation() - pawn->GetActorLocation();
		float Distance = Line.Size();
		float MaxDistance = (PawnSensingComp->SightRadius / 2) / FMath::Tan(PawnSensingComp->GetPeripheralVisionAngle() / 2);

		float Angle = FVector::DotProduct(GetActorForwardVector(), Line);

		if (Distance < MaxDistance &&
		Angle < - (1.f - PawnSensingComp->GetPeripheralVisionAngle() / 90.f)) {
			return true;
		}
		return false;
	}
	return false;
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
			Destroy();
			return (DamageAmount - Health);
		}
	}
	return 0.f;
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "MyAICharacter.h"
#include "AnAIController.h"
#include "CodeAICharacter.h"
#include "PlayerHUD.h"
#include "MyPlayerController.h"
#include "BotTargetPoint.h"
#include "WeaponItem.h"
#include "AudioManager.h"
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

	MaxHealth = 100.f;
	Health = 100.f;
	MinAttackRate = 1.f;
	MaxAttackRate = 1.5f;
	ExtraTime = FMath::RandRange(MinAttackRate, MaxAttackRate);
	Time = ExtraTime / 2;

	bCanSeePlayer = false;
	bHeardNoise = false;
	bHeldUp = false;
	bSightBlockedByCover = false;
	bReverseTargetPoints = false;
	bIsAiming = false;
	bPlayerDead = false;
	bIsDead = false;
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

	//Get a reference to the AudioManager
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAudioManager::StaticClass(), AudioFetcher);
	if (AudioFetcher.Num() > 0) {
		AudioMan = Cast<AAudioManager>(AudioFetcher[0]);
	}
}

void AMyAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//If the player isn't dead
	if (!bPlayerDead) {
		if (!bIsDead) {
			//If the pawn can sense anything, stop chasing the player
			if (bCanSeePlayer) {
				Time += DeltaTime;
				SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ChasingPawn->GetActorLocation()));
				if (Time >= ExtraTime) {
					Time = 0.f;
					ExtraTime = FMath::RandRange(MinAttackRate, MaxAttackRate);
					ShootWeapon();
					AimWeapon();
				}
				if (!CheckLineOfSightTo(ChasingPawn)) {

					AAnAIController* AIController = Cast<AAnAIController>(GetController());
					if (AIController) {
						AIController->SetPlayerLocation(ChasingPawn->GetActorLocation());
						AIController->SetState(EAIState::AI_ChasingTarget);
						AIController->ClearSeenTarget();
						LowerWeapon();
						ChasingPawn->RemoveEnemy(this);
						bCanSeePlayer = false;
						bSightBlockedByCover = false;
						Time = 0.f;
						ExtraTime = FMath::RandRange(MinAttackRate, MaxAttackRate);
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
			else if (CheckLineOfSightTo(ChasingPawn)) {
				PlayerSeen();
			}
		}
	}
}

void AMyAICharacter::OnSeePlayer(APawn* Pawn)
{
	if (!bIsDead) {
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
}

void AMyAICharacter::PlayerSeen()
{
	if (!ChasingPawn->IsDead()) {
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
					FVector CheckVector = GetActorLocation() - ChasingPawn->GetActorLocation();
					CheckVector.Normalize();
					float Check = FVector::DotProduct(ChasingPawn->GetActorForwardVector(), CheckVector);

					if (Dot <= -0.4f || Check > -0.1f) {
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
}

void AMyAICharacter::OnHearNoise(APawn * PawnInstigator, const FVector & Location, float Volume)
{
	if (!bIsDead) {
		AAnAIController* AIController = Cast<AAnAIController>(GetController());
		if (AIController && (AIController->GetState() != EAIState::AI_HeldUp)) {

			AMyAICharacter* AICharacter = Cast<AMyAICharacter>(PawnInstigator);
			ACodeAICharacter* Player = Cast<ACodeAICharacter>(PawnInstigator);
			//If the noise came from a friendly AI Character
			if (AICharacter) {
				AIController->SetHelpBot(Cast<APawn>(AICharacter));
				AIController->SetState(EAIState::AI_HelpingFriendly);
				AIController->SetSlowSpeed(false);
			}
			//If the noise came from the player
			else if (!bCanSeePlayer && Player) {
				FVector Len = GetActorLocation() - Location;
				//If the player is too close for comfort
				if (Len.Size() <= MinHearingThresh && !Player->InCover()) {
					DetectPlayer(AIController, PawnInstigator);
				}
				else {
					AIController->SetHeardLocation(Location);
					//If the AI isn't already chasing a sound, it is now
					if (AIController->GetState() != EAIState::AI_ChasingSound) {
						AIController->SetState(EAIState::AI_ChasingSound);
					}
					AIController->SetSlowSpeed(true);
					//If there is a sound, play it
					if (SoundHeardSound && !bHeardNoise) {
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundHeardSound, GetActorLocation());
					}
					bHeardNoise = true;
				}
			}
		}
	}
}

void AMyAICharacter::DetectPlayer(class AAnAIController* AICon, APawn * Pawn)
{
	if (AICon->GetState() != EAIState::AI_HeldUp) {
		//If there is a sound and the AI isn't already chasing the player, play it
		if (PlayerFoundSound && AICon->GetState() != EAIState::AI_ChasingTarget) {
			ReportNoise(PlayerFoundSound, 1.f);
		}
		AICon->SetSeenTarget(Pawn);
		AICon->SetState(EAIState::AI_AttackingTarget);
		AICon->SetSlowSpeed(false);
		bCanSeePlayer = true;
		AimWeapon();
		ChasingPawn->AddEnemy(this);
		if (AudioMan) {
			AudioMan->IncrementEnemies();
		}
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
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, GetActorLocation(), 1.f);

		if (Volume < 4.f) {
			PawnNoiseEmitterComp->MakeNoise(this, Volume, GetActorLocation());
		}
		else if (Weapon){
			ChasingPawn->NotifyLoudNoise(GetActorLocation());
		}
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
		Weapon->UseItemPressed(false);
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
		Weapon->UseItemReleased(true);
	}
}

void AMyAICharacter::SufferHoldUp()
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController &&
		AIController->GetState() != EAIState::AI_HeldUp &&
		AIController->GetState() != EAIState::AI_AttackingTarget &&
		AIController->GetState() != EAIState::AI_ChasingTarget) {
			
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%d"), (uint8)AIController->GetState()));
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
		FVector Line = LineAux;
		Line.Normalize();

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

void AMyAICharacter::PlayerKilled()
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController && AIController->GetState() != EAIState::AI_HeldUp) {
		AIController->SetState(EAIState::AI_Patrolling);
		AIController->ClearSeenTarget();
		AIController->SetSlowSpeed(true);
		LowerWeapon();
		bPlayerDead = true;
	}
}

void AMyAICharacter::RunToHeardSound(FVector Loc)
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController &&
		AIController->GetState() == EAIState::AI_Patrolling) {
			AIController->SetHeardLocation(Loc);
			//If the AI isn't already chasing a sound, it is now
			if (AIController->GetState() != EAIState::AI_ChasingSound) {
				AIController->SetState(EAIState::AI_ChasingSound);
			}
			AIController->SetSlowSpeed(false);
			//If there is a sound, play it
			if (SoundHeardSound && !bHeardNoise) {
				ReportNoise(SoundHeardSound, 1.f);
			}
			bHeardNoise = true;
	}
}

EAIState AMyAICharacter::GetCurrentState() const
{
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController) {
		return AIController->GetState();
	}
	else {
		return EAIState();
	}
}

void AMyAICharacter::SetRunning(bool IsRunning)
{
	bIsRunning = IsRunning;
}

void AMyAICharacter::SetNoiseHeard(bool bHeard)
{
	bHeardNoise = bHeard;
}

void AMyAICharacter::OnDeath()
{
	//Set new state
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (AIController) {
		AIController->SetState(EAIState::AI_Dead);
	}
	//Play sound
	if (KilledSound) {
		ReportNoise(KilledSound, 1.f);
	}
	//Remove this bot from the player's radar
	APlayerHUD* HUD = Cast<APlayerHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
	if (HUD) {
		HUD->RemoveFromRadar(this);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (DeathAnimations.Num() > 0) {
		int Random = FMath::RandRange(0, DeathAnimations.Num() - 1);
		GetMesh()->PlayAnimation(DeathAnimations[Random], false);
	}
	bCanSeePlayer = false;
	bIsDead = true;
	LowerWeapon();
	if (AudioMan) {
		AudioMan->DecrementEnemies();
	}
}

void AMyAICharacter::OnDestroy()
{
	if (Weapon) {
		Weapon->Destroy();
	}
	Destroy();
}

float AMyAICharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	AAnAIController* AIController = Cast<AAnAIController>(GetController());
	if (!bCanSeePlayer && AIController &&
		(AIController->GetState() == EAIState::AI_Patrolling ||
		AIController->GetState() == EAIState::AI_HeldUp)) {
			AMyPlayerController* PC = Cast<AMyPlayerController>(EventInstigator);
			if (PC) {
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
			Health = 0.f;
			OnDeath();
			return (DamageAmount - Health);
		}
	}
	return 0.f;
}
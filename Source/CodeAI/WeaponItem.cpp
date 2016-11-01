// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "WeaponItem.h"
#include "MyAICharacter.h"
#include "CodeAICharacter.h"

AWeaponItem::AWeaponItem()
{
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	GunMesh->SetupAttachment(RootComponent);
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetVisibility(false);

	ClipSize = 0;
	MaxAmmo = 20;
	ClipAmount = ClipSize;
	MagsAmount = 0;
	HoldUpDistance = 250.f;

	bIsAimed = false;
	bCancelled = false;
}

void AWeaponItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsAimed) {
		FVector Start = GunMesh->GetSocketLocation(TEXT("MuzzleSocket"));
		FVector Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorForwardVector();
		FVector End = Start + FVector(Player.X, Player.Y, 0.f)* 1000.f;
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.f, 0, 2.f);
	}
}

void AWeaponItem::AddAmmo(int32 NewQuantity)
{
	if (!AmmoIsFull()) {
		if ((ClipAmount + MagsAmount + MaxAmmo) < MaxAmmo) {
			MagsAmount += NewQuantity;
		}
		else {
			MagsAmount = MaxAmmo - ClipAmount;
		}
	}
}

void AWeaponItem::DecrementAmmo()
{
	ClipAmount--;
}

void AWeaponItem::Reload()
{
	if ((ClipSize - ClipAmount) != 0 && MagsAmount != 0) {
		int ammo = ClipSize - ClipAmount;
		ClipAmount += ammo;
		MagsAmount -= ammo;
	}
}

void AWeaponItem::AIEquip(class AMyAICharacter* AIChar)
{
	AIChar->AddWeapon(this);
	SM_ItemPickup->SetVisibility(false);
	SM_ItemPickup->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BC_BoxComp->bGenerateOverlapEvents = false;
	GunMesh->SetVisibility(true);
}

void AWeaponItem::UseItemPressed(APawn* Pawn)
{
	bIsAimed = true;
	ACodeAICharacter* Char = Cast<ACodeAICharacter>(Pawn);
	if (Char) {
		TArray<FHitResult> Hits;
		FVector Start = GunMesh->GetSocketLocation(TEXT("MuzzleSocket"));
		FVector End = Start + Char->GetActorForwardVector() * HoldUpDistance;
		GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(HoldUpDistance));
		DrawDebugSphere(GetWorld(), Start, HoldUpDistance, 50, FColor::Green, true);

		for (FHitResult nHit : Hits) {
			AMyAICharacter* AI = Cast<AMyAICharacter>(nHit.GetActor());
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Found Bot"));
			if (AI) { 
				AI->SufferHoldUp();
			}
		}
	}
}

void AWeaponItem::UseItemReleased(APawn* Pawn)
{
	if (!bCancelled) {
		bIsAimed = false;
		if (ClipIsEmpty()) {
			if (OutOfAmmoSound) {
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), OutOfAmmoSound, GetActorLocation(), 1.f);
			}
			return;
		}
		else {
			if (GunShotSound) {
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), GunShotSound, GetActorLocation(), 1.f);
			}
			DecrementAmmo();
			ACodeAICharacter* Char = Cast<ACodeAICharacter>(Pawn);
			if (Char) {
				FVector Start = GunMesh->GetSocketLocation(TEXT("MuzzleSocket"));
				FVector Player = Char->GetActorForwardVector();
				FVector End = Start + FVector(Player.X, Player.Y, 0.f)* 1000.f;
				FHitResult Hit;

				if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Pawn)) {
					FPointDamageEvent DamageEvent;
					AMyAICharacter* AI = Cast<AMyAICharacter>(Hit.GetActor());
					if (AI) {
						AI->TakeDamage(Char->GetDamage(), DamageEvent, Char->GetController(), this);
					}
				}
			}
			else {
				AMyAICharacter* AI = Cast<AMyAICharacter>(Pawn);
				if (AI) {
					FVector Start = GunMesh->GetSocketLocation(TEXT("MuzzleSocket"));
					FVector Bot = AI->GetActorForwardVector();
					FVector End = Start + FVector(Bot.X, Bot.Y, 0.f)* 1000.f;
					FHitResult Hit;

					if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Pawn)) {
						FPointDamageEvent DamageEvent;
						ACodeAICharacter* Character = Cast<ACodeAICharacter>(Hit.GetActor());
						if (Character) {
							Character->TakeDamage(AI->GetDamage(), DamageEvent, AI->GetController(), this);
						}
					}
				}
			}
		}
	}
	else {
		bCancelled = false;
	}
}

void AWeaponItem::CancelUse()
{
	if (bIsAimed) {
		bIsAimed = false;
		bCancelled = true;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Cancelled"));
	}
}

UStaticMeshComponent * AWeaponItem::GetGunMesh() const
{
	return GunMesh;
}

void AWeaponItem::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}
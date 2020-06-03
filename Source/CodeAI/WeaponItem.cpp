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
	HoldUpDistance = 125.f;

	bIsAimed = false;
	bCancelled = false;

	bIsSilenced = false;
}

void AWeaponItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsAimed) {
		FVector Start = GunMesh->GetSocketLocation(TEXT("MuzzleSocket"));
		FVector Player = Owner->GetActorForwardVector();
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
		int Ammo = ClipSize - ClipAmount;
		if (Ammo <= MagsAmount) {
			ClipAmount += Ammo;
			MagsAmount -= Ammo;
		}
		else {
			ClipAmount += MagsAmount;
			MagsAmount -= MagsAmount;
		}
	}
}

void AWeaponItem::SetOwner(AActor* NewOwner)
{
	Owner = NewOwner;
}

void AWeaponItem::AIEquip(class AMyAICharacter* AIChar)
{
	AIChar->AddWeapon(this);
	SM_ItemPickup->SetVisibility(false);
	SM_ItemPickup->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BC_BoxComp->SetGenerateOverlapEvents(false);
	GunMesh->SetVisibility(true);
	Owner = AIChar;
}

void AWeaponItem::UseItemPressed(bool bShouldHoldUp)
{
	bIsAimed = true;
	if (Owner && bShouldHoldUp) {
		if (!bIsRifle) {
			TArray<FHitResult> Hits;
			FVector Start = GunMesh->GetSocketLocation(TEXT("MuzzleSocket"));
			FVector End = Start + Owner->GetActorForwardVector() * HoldUpDistance;
			FCollisionQueryParams TraceParams(FName(TEXT("OverlapMulti Trace")), false);

			GetWorld()->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(HoldUpDistance), TraceParams);
			for (FHitResult nHit : Hits) {
				AMyAICharacter* AI = Cast<AMyAICharacter>(nHit.GetActor());
				if (AI) {
					AI->SufferHoldUp();
					continue;
				}
			}
		}
		else {
			RifleShoot();
			GetWorldTimerManager().SetTimer(FireRateHandle, this, &AWeaponItem::RifleShoot, 1 / (FireRate / 60.f), true);
		}
	}
}

void AWeaponItem::UseItemReleased(bool bShouldStop)
{
	if (!bCancelled && !bIsRifle || (bIsRifle && !bShouldStop)) {
		if (!bIsRifle) {
			bIsAimed = false;
		}
		if (ClipIsEmpty()) {
			if (OutOfAmmoSound) {
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), OutOfAmmoSound, GetActorLocation(), 1.f);
			}
			return;
		}
		else {
			DecrementAmmo();
			ACodeAICharacter* Char = Cast<ACodeAICharacter>(Owner);
			if (Char) {
				FVector Start = GunMesh->GetSocketLocation(TEXT("MuzzleSocket"));
				FVector Player = Char->GetActorForwardVector();
				FVector End = Start + FVector(Player.X, Player.Y, 0.f)* 1000.f;
				FHitResult Hit;
				FCollisionQueryParams QParams;
				QParams.AddIgnoredActor(this);
				if (!bIsSilenced) {
					if (GunShotSound) {
						Char->ReportNoise(GunShotSound, 4.f);
					}
				}
				else if (GunShotSoundSilenced) {
					UGameplayStatics::PlaySound2D(GetWorld(), GunShotSoundSilenced);
				}

				if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Pawn)) {
					FPointDamageEvent DamageEvent;
					AMyAICharacter* AI = Cast<AMyAICharacter>(Hit.GetActor());
					if (AI) {
						AI->TakeDamage(Damage, DamageEvent, Char->GetController(), this);
					}
				}
			}
			else {
				AMyAICharacter* AI = Cast<AMyAICharacter>(Owner);
				if (AI) {
					FVector Start = GunMesh->GetSocketLocation(TEXT("MuzzleSocket"));
					FVector Bot = AI->GetActorForwardVector();
					FVector End = Start + FVector(Bot.X, Bot.Y, 0.f)* 1000.f;
					FHitResult Hit;
					if (!bIsSilenced) {
						if (GunShotSound) {
							AI->ReportNoise(GunShotSound, 4.f);
						}
					}
					else if (GunShotSoundSilenced) {
						UGameplayStatics::PlaySound2D(GetWorld(), GunShotSoundSilenced);
					}

					if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Pawn)) {
						FPointDamageEvent DamageEvent;
						ACodeAICharacter* Character = Cast<ACodeAICharacter>(Hit.GetActor());
						if (Character) {
							Character->TakeDamage(Damage, DamageEvent, AI->GetController(), this);
						}
					}
				}
			}
		}
	}
	else if (bCancelled || bIsRifle && bShouldStop){
		bCancelled = false;
		bIsAimed = false;
		GetWorldTimerManager().ClearTimer(FireRateHandle);
	}
}

void AWeaponItem::RifleShoot()
{
	UseItemReleased(false);
}

void AWeaponItem::CancelUse()
{
	if (bIsAimed) {
		bIsAimed = false;
		bCancelled = true;
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
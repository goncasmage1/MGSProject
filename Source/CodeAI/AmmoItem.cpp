// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "AmmoItem.h"
#include "CodeAICharacter.h"

AAmmoItem::AAmmoItem()
{
	WeaponName = "Ammo Item";
}

void AAmmoItem::BeginPlay()
{
	Super::BeginPlay();
}

void AAmmoItem::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

FName AAmmoItem::GetWeaponName()
{
	return WeaponName;
}

int32 AAmmoItem::GetQuantity()
{
	return Quantity;
}

void AAmmoItem::AddQuantity(int32 NewQuantity)
{
	Quantity += NewQuantity;
}

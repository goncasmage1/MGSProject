// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "InventoryGameItem.h"
#include "CodeAICharacter.h"

AInventoryGameItem::AInventoryGameItem()
{
	bIsGadget = false;
}

void AInventoryGameItem::BeginPlay()
{
	Super::BeginPlay();
}

void AInventoryGameItem::UseItemPressed(bool bShouldHoldUp)
{

}

void AInventoryGameItem::UseItemReleased(bool bShouldStop)
{

}

void AInventoryGameItem::Equip_Implementation()
{

}

void AInventoryGameItem::Unequip_Implementation()
{

}

void AInventoryGameItem::MenuUse_Implementation()
{

}

void AInventoryGameItem::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

UTexture2D* AInventoryGameItem::GetItemTexture()
{
	return ItemImage;
}

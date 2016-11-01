// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "StackableGameItem.h"

int32 AStackableGameItem::GetQuantity()
{
	return Quantity;
}

void AStackableGameItem::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	Super::OnOverlapBegin(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void AStackableGameItem::AddQuantity(int32 NewQuantity)
{
	Quantity += NewQuantity;
}

void AStackableGameItem::DecreaseQuantity(int32 NewQuantity)
{
	if ((Quantity - NewQuantity) >= 0) {
		Quantity -= NewQuantity;
	}
}
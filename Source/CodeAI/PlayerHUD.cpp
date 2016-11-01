// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "PlayerHUD.h"
#include "Kismet/KismetMathLibrary.h"

void APlayerHUD::AddToRadar(AActor * NewActor)
{
	RadarArray.Add(NewActor);
}

void APlayerHUD::RemoveFromRadar(AActor * Actor)
{
	RadarArray.Remove(Actor);
}

FVector2D APlayerHUD::GetRadarCenterPosition()
{
	return FVector2D(ScreenSize.X * RadarStartLocation.X, ScreenSize.Y * RadarStartLocation.Y);
}

FVector2D APlayerHUD::GetRadarDotPosition(FVector Location)
{
	FTransform Trans = FTransform(FRotator(0.f, 0.f, 0.f),
								GetOwningPlayerController()->GetPawn()->GetActorLocation(),
								GetOwningPlayerController()->GetPawn()->GetActorScale3D());
	FVector ITL = UKismetMathLibrary::InverseTransformLocation(Trans, Location);
	ITL /= ObjectDistance;

	return FVector2D(ITL.X, ITL.Y);
}

void APlayerHUD::DrawBorder()
{
	DrawLine(GetRadarCenterPosition().X - RadarSize,
		GetRadarCenterPosition().Y - RadarSize,
		GetRadarCenterPosition().X - RadarSize,
		GetRadarCenterPosition().Y + RadarSize,
		FColor::Black, 2.f);
	DrawLine(GetRadarCenterPosition().X - RadarSize,
		GetRadarCenterPosition().Y + RadarSize,
		GetRadarCenterPosition().X + RadarSize,
		GetRadarCenterPosition().Y + RadarSize,
		FColor::Black, 2.f);
	DrawLine(GetRadarCenterPosition().X + RadarSize,
		GetRadarCenterPosition().Y + RadarSize,
		GetRadarCenterPosition().X + RadarSize,
		GetRadarCenterPosition().Y - RadarSize,
		FColor::Black, 2.f);
	DrawLine(GetRadarCenterPosition().X + RadarSize,
		GetRadarCenterPosition().Y - RadarSize,
		GetRadarCenterPosition().X - RadarSize,
		GetRadarCenterPosition().Y - RadarSize,
		FColor::Black, 2.f);
}

void APlayerHUD::ContinueDrawHUD(int32 SizeX, int32 SizeY)
{
	ScreenSize.X = SizeX;
	ScreenSize.Y = SizeY;

	DrawBorder();

	DrawRect(FColor::Blue, GetRadarCenterPosition().X, GetRadarCenterPosition().Y, 5.f, 5.f);

	for (AActor* Actor : RadarArray) {
		float x, x1, y, y1;
		//Math to draw the actor's location
		x = - GetRadarDotPosition(Actor->GetActorLocation()).X;
		y = GetRadarDotPosition(Actor->GetActorLocation()).Y;
		if (x < -RadarSize || x > RadarSize || y < -RadarSize || y > RadarSize) {

		}
		else {
			//Math to draw the actor's line of sight
			FVector Forward = Actor->GetActorLocation() + Actor->GetActorForwardVector() * 550;
			x1 = -GetRadarDotPosition(Forward).X;
			y1 = GetRadarDotPosition(Forward).Y;

			DrawLine(y + GetRadarCenterPosition().X + 2.5f,
				x + GetRadarCenterPosition().Y + 2.5f,
				y1 + GetRadarCenterPosition().X + 2.5f,
				x1 + GetRadarCenterPosition().Y + 2.5f,
				FColor::Blue);

			DrawRect(FColor::Black,
				y + GetRadarCenterPosition().X,
				x + GetRadarCenterPosition().Y,
				5.f, 5.f);
		}
	}
}




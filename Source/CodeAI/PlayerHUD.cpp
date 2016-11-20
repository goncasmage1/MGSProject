// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "PlayerHUD.h"
#include "CornerManager.h"
#include "Kismet/KismetMathLibrary.h"

void APlayerHUD::BeginPlay()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACornerManager::StaticClass(), Corners);
	ACornerManager* Man = Cast<ACornerManager>(Corners[0]);
	if (Man) {
		Manager = Man;
	}
}

void APlayerHUD::AddToRadar(AActor * NewActor)
{
	EnemiesArray.Add(NewActor);
}

void APlayerHUD::RemoveFromRadar(AActor * Actor)
{
	EnemiesArray.Remove(Actor);
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

void APlayerHUD::FitToRadar(float &x, float &y)
{
	if (x < -RadarSize) {
		x = -RadarSize;
	}
	else if (x > RadarSize) {
		x = RadarSize;
	}
	if (y < -RadarSize) {
		y = -RadarSize;
	}
	else if (y > RadarSize) {
		y = RadarSize;
	}
}

bool APlayerHUD::IsInRadar(float x, float y) const
{
	return !(x < -RadarSize || x > RadarSize || y < -RadarSize || y > RadarSize);
}

void APlayerHUD::DrawBase()
{
	DrawRect(RadarBaseColor, RCPX - RadarSize, RCPY - RadarSize, RadarSize * 2, RadarSize * 2);
}

void APlayerHUD::DrawBorder()
{
	DrawLine(RCPX - RadarSize,
			RCPY - RadarSize,
			RCPX - RadarSize,
			RCPY + RadarSize,
			FColor::Black, 2.f);
	DrawLine(RCPX - RadarSize,
			RCPY + RadarSize,
			RCPX + RadarSize,
			RCPY + RadarSize,
			FColor::Black, 2.f);
	DrawLine(RCPX + RadarSize,
			RCPY + RadarSize,
			RCPX + RadarSize,
			RCPY - RadarSize,
			FColor::Black, 2.f);
	DrawLine(RCPX + RadarSize,
			RCPY - RadarSize,
			RCPX - RadarSize,
			RCPY - RadarSize,
			FColor::Black, 2.f);
}

void APlayerHUD::DrawEnemies()
{
	for (AActor* Actor : EnemiesArray) {
		if (Actor->IsValidLowLevel() && !Actor->IsPendingKillPending()) {
			float x, y;
			//Math to draw the actor's location
			x = -GetRadarDotPosition(Actor->GetActorLocation()).X;
			y = GetRadarDotPosition(Actor->GetActorLocation()).Y;

			//If the enemy is outside the radar, don't draw it
			if (IsInRadar(x, y)) {
				DrawRect(FColor::Black,
					y + RCPX,
					x + RCPY,
					5.f, 5.f);
				if (EnemySight) {
					FLinearColor Color = FLinearColor::Blue;
					Color.A = .3f;
					DrawTexture(EnemySight,
						y + RCPX - ((EnemySight->GetSizeX() * .45f) / 2 - 2.5f),
						x + RCPY - ((EnemySight->GetSizeY() * .45f) / 2 - 2.5f),
						EnemySight->GetSizeX(),
						EnemySight->GetSizeY(),
						EnemySight->GetSizeX(),
						EnemySight->GetSizeY(),
						1.f,
						1.f,
						Color,
						EBlendMode::BLEND_Translucent,
						.45f,
						false,
						Actor->GetActorRotation().Yaw,
						FVector2D(0.5f, 0.5f));
				}
			}
		}
	}
}

void APlayerHUD::DrawCubes()
{
	for (FCornerStruct CornerSet : Manager->Corners) {
		int Max = CornerSet.CornerArray.Num();
		int i;
		for (i = 1; i < Max; i++) {
			float x1 = -GetRadarDotPosition(CornerSet.CornerArray[i - 1]->GetActorLocation()).X;
			float y1 = GetRadarDotPosition(CornerSet.CornerArray[i - 1]->GetActorLocation()).Y;
			float x2 = -GetRadarDotPosition(CornerSet.CornerArray[i]->GetActorLocation()).X;
			float y2 = GetRadarDotPosition(CornerSet.CornerArray[i]->GetActorLocation()).Y;
			float X1 = y1 + RCPX;
			float Y1 = x1 + RCPY;
			float X2 = y2 + RCPX;
			float Y2 = x2 + RCPY;
				
			if (IsInRadar(x1, y1) || IsInRadar(x2, y2)) {
				if (IsInRadar(x1, y1) && IsInRadar(x2, y2)) {
					DrawLine(X1, Y1, X2, Y2, FLinearColor::Green);
				}
				else if (IsInRadar(x1, y1)) {
					FitToRadar(x2, y2);
					DrawLine(X1, Y1,
							y2 + RCPX,
							x2 + RCPY,
							FLinearColor::Green);
				}
				else if (IsInRadar(x2, y2)) {
					FitToRadar(x1, y1);
					DrawLine(y1 + RCPX,
							x1 + RCPY,
							X2,	Y2,	FLinearColor::Green);
				}
			}
			if (i == (Max - 1) && CornerSet.bConnectFirstToLast) {
				float x3 = -GetRadarDotPosition(CornerSet.CornerArray[i]->GetActorLocation()).X;
				float y3 = GetRadarDotPosition(CornerSet.CornerArray[i]->GetActorLocation()).Y;
				float x4 = -GetRadarDotPosition(CornerSet.CornerArray[0]->GetActorLocation()).X;
				float y4 = GetRadarDotPosition(CornerSet.CornerArray[0]->GetActorLocation()).Y;
				float X3 = y3 + RCPX;
				float Y3 = x3 + RCPY;
				float X4 = y4 + RCPX;
				float Y4 = x4 + RCPY;
				if (IsInRadar(x3, y3) || IsInRadar(x4, y4)) {
					if (IsInRadar(x3, y3) && IsInRadar(x4, y4)) {
						DrawLine(X3, Y3, X4, Y4, FLinearColor::Green);
					}
					else if (IsInRadar(x3, y3)) {
						FitToRadar(x4, y4);
						DrawLine(X3, Y3,
							y4 + RCPX,
							x4 + RCPY,
							FLinearColor::Green);
					}
					else if (IsInRadar(x4, y4)) {
						FitToRadar(x3, y3);
						DrawLine(y3 + RCPX,
							x3 + RCPY,
							X4, Y4, FLinearColor::Green);
					}
				}
			}
		}
	}
}

void APlayerHUD::ContinueDrawHUD(int32 SizeX, int32 SizeY)
{
	RCPX = SizeX * RadarStartLocation.X;
	RCPY = SizeY * RadarStartLocation.Y;

	DrawBase();
	DrawCubes();
	DrawEnemies();
	DrawRect(FColor::Blue, RCPX, RCPY, 5.f, 5.f);
	DrawBorder();
	
}




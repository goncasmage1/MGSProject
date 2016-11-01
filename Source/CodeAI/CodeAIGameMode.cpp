// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CodeAI.h"
#include "CodeAIGameMode.h"
#include "CodeAICharacter.h"
#include "PlayerHUD.h"

ACodeAIGameMode::ACodeAIGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/MyPlayerController_BP"));
	if (PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
	
	static ConstructorHelpers::FClassFinder<AHUD> HUDBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/MyPlayerHUD"));
	if (HUDBPClass.Class != NULL)
	{
		HUDClass = HUDBPClass.Class;
	}
	
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "CornerManager.h"


// Sets default values
ACornerManager::ACornerManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACornerManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACornerManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}


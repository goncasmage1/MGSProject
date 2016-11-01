
// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "MGSCube.h"


// Sets default values
AMGSCube::AMGSCube()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
	RootComponent = SceneComp;

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube Mesh"));
	CubeMesh->SetupAttachment(SceneComp);
}


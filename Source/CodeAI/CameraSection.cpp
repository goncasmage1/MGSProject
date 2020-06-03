// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "CameraSection.h"
#include "CodeAICharacter.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ACameraSection::ACameraSection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
	RootComponent = SceneComp;

	BC_BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	BC_BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ACameraSection::OnOverlapBegin);
	BC_BoxComp->OnComponentEndOverlap.AddDynamic(this, &ACameraSection::OnOverlapEnd);
	BC_BoxComp->SetupAttachment(SceneComp);
	BC_BoxComp->SetGenerateOverlapEvents(true);

	bFollowPlayer = false;

}

// Called when the game starts or when spawned
void ACameraSection::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACameraSection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Character && bFollowPlayer)
	{
		CamActor->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(CamActor->GetActorLocation(), Character->GetActorLocation()));
	}
}

void ACameraSection::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (CamActor)
	{
		ACodeAICharacter* Char = Cast<ACodeAICharacter>(OtherActor);
		if (Char && CamActor)
		{
			Character = Char;
			Char->EnterCameraSection(CamActor);
		}
	}
}

void ACameraSection::OnOverlapEnd(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	if (CamActor)
	{
		ACodeAICharacter* Char = Cast<ACodeAICharacter>(OtherActor);
		if (Char)
		{
			Char->ExitCameraSection(CamActor);
			Character = nullptr;
		}
	}
}


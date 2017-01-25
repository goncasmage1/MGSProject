// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "AudioManager.h"


// Sets default values
AAudioManager::AAudioManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AudioComp_1 = CreateDefaultSubobject<UAudioComponent>("AudioComponent 1");
	AudioComp_2 = CreateDefaultSubobject<UAudioComponent>("AudioComponent 2");

	EnemiesNum = 0;
}

// Called when the game starts or when spawned
void AAudioManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (SoundTracks.Num() > 0) {
		AudioComp_1->Sound = SoundTracks[0];
		AudioComp_1->Play();
	}
}

void AAudioManager::OnPlayerFound()
{
	if (SoundTracks.Num() > 1) {
		AudioComp_1->Stop();
		AudioComp_2->Sound = SoundTracks[1];
		AudioComp_2->Play();
	}
}

void AAudioManager::OnPlayerLost()
{
	if (SoundTracks.Num() > 0) {
		AudioComp_2->FadeOut(.75f, 0.f);
		AudioComp_1->Sound = SoundTracks[0];
		AudioComp_1->FadeIn(1.f);
	}
}

void AAudioManager::IncrementEnemies()
{
	if (EnemiesNum == 0) {
		OnPlayerFound();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Player Attacked"));
	}
	EnemiesNum++;
}

void AAudioManager::DecrementEnemies()
{
	if (EnemiesNum > 0) {
		EnemiesNum--;
		if (EnemiesNum == 0) {
			OnPlayerLost();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Player Lost"));
		}
	}
}


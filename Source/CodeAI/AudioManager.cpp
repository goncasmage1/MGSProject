// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "AudioManager.h"
#include "MyAICharacter.h"


// Sets default values
AAudioManager::AAudioManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AudioComp_1 = CreateDefaultSubobject<UAudioComponent>("AudioComponent 1");
	AudioComp_2 = CreateDefaultSubobject<UAudioComponent>("AudioComponent 2");
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
		AudioComp_2->FadeOut(0.75f, 0.f);
		AudioComp_1->Sound = SoundTracks[0];
		AudioComp_1->FadeIn(2.f);
	}
}

void AAudioManager::AddEnemy(AMyAICharacter* AIChar)
{
	if (Enemies.Num() == 0) {
		OnPlayerFound();
	}
	Enemies.AddUnique(AIChar);
}

void AAudioManager::RemoveEnemy(AMyAICharacter* AIChar)
{
	if (Enemies.Num() > 0) {
		Enemies.Remove(AIChar);
		if (Enemies.Num() == 0) {
			OnPlayerLost();
		}
	}
}


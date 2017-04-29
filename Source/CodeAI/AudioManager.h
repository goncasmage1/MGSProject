// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "AudioManager.generated.h"

UCLASS()
class CODEAI_API AAudioManager : public AActor
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere)
		UAudioComponent* AudioComp_1;
	UPROPERTY(VisibleAnywhere)
		UAudioComponent* AudioComp_2;
	UPROPERTY(EditAnywhere)
		TArray<USoundBase*> SoundTracks;

	TArray<class AMyAICharacter*> Enemies;
	
public:	

	// Sets default values for this actor's properties
	AAudioManager();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Called when the player is found by the enemy
	void OnPlayerFound();
	//Called when the player is no longer in view of the enemy
	void OnPlayerLost();

	//Called when an enemy spots the player
	void AddEnemy(class AMyAICharacter* AIChar);
	//Called when an enemy loses the player from his sight
	void RemoveEnemy(class AMyAICharacter* AIChar);
	
};

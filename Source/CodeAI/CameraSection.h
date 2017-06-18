// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "CameraSection.generated.h"

UCLASS()
class CODEAI_API ACameraSection : public AActor
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere)
		USceneComponent* SceneComp;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent *BC_BoxComp;

	class ACodeAICharacter* Character;
	
public:

	ACameraSection();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		bool bFollowPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	ACameraActor* CamActor;

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GameItem.generated.h"

UCLASS()
class CODEAI_API AGameItem : public AActor
{
	GENERATED_BODY()
	
public:	

	AGameItem();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;

	void DestroyComponents();
	FName GetItemName() const;


protected:

	//Used as a root component to keep things properly attached
	UPROPERTY(VisibleAnywhere)
		USceneComponent* SceneComp;
	//Mesh of the item in the level (a box)
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent *SM_ItemPickup;
	//Collision component for the mesh
	UPROPERTY(VisibleAnywhere)
		UBoxComponent *BC_BoxComp;
	//The widget of text that pops up when the item is picked up
	UPROPERTY(VisibleAnywhere)
		class UWidgetComponent* TextWidget;
	//The item's name
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName ItemName;
	//Sound to play when the item was picked up
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		USoundBase* ItemPickedUpSound;
	//Sound to play when the item has failed to be picked up
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		USoundBase* ItemFailedSound;

	//Used to add a rotation on tick
	float RotationRate;
	//Amount of time the item text widget stays on the screen
	UPROPERTY(EditDefaultsOnly)
		float ItemTextExpiration;

	//Timer handle for the item widget
	FTimerHandle ItemNameHandle;

	//OnComponentBeginOverlap function
	UFUNCTION()
		virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//Called when the time of the handle has expired
	void HideItemText();
	
};

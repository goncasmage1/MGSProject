// Fill out your copyright notice in the Description page of Project Settings.

#include "CodeAI.h"
#include "GameItem.h"
#include "CodeAICharacter.h"
#include "MyAICharacter.h"
#include "ItemTextWidget.h"
#include "Components/WidgetComponent.h"

// Sets default values
AGameItem::AGameItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
	RootComponent = SceneComp;
	
	SM_ItemPickup = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Pickup Mesh"));
	SM_ItemPickup->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SM_ItemPickup->SetupAttachment(RootComponent);

	BC_BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Item Collision Box"));
	BC_BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGameItem::OnOverlapBegin);
	BC_BoxComp->SetupAttachment(SM_ItemPickup);
	BC_BoxComp->bGenerateOverlapEvents = true;

	TextWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Text Widget"));
	TextWidget->SetWidgetSpace(EWidgetSpace::Screen);
	TextWidget->SetVisibility(false);

	ItemName = "Game Item";

	RotationRate = 1.5f;
	ItemTextExpiration = 1.5f;
}

// Called when the game starts or when spawned
void AGameItem::BeginPlay()
{
	Super::BeginPlay();
	TextWidget->SetVisibility(false);
}

// Called every frame
void AGameItem::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	SM_ItemPickup->AddRelativeRotation(FRotator(0.f, RotationRate, 0.f));

}

FName AGameItem::GetItemName ()const
{
	return ItemName;
}

void AGameItem::DestroyComponents()
{
	SM_ItemPickup->DestroyComponent();
	BC_BoxComp->DestroyComponent();
}

void AGameItem::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	ACodeAICharacter* Char = Cast<ACodeAICharacter>(OtherActor);
	if (Char)
	{
		bool Success = Char->AddItem(this);
		TextWidget->SetVisibility(true);

		if (Success) {
			SM_ItemPickup->SetVisibility(false);
			SM_ItemPickup->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			BC_BoxComp->bGenerateOverlapEvents = false;
			if (ItemPickedUpSound) {
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ItemPickedUpSound, GetActorLocation());
			}
		}
		else if (ItemFailedSound){
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ItemFailedSound, GetActorLocation());
		}
	}
	else {
		AMyAICharacter* AI = Cast<AMyAICharacter>(OtherActor);
		if (AI) {
			bool Success = Char->AddItem(this);
			if (Success) {
				SM_ItemPickup->SetVisibility(false);
				SM_ItemPickup->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				BC_BoxComp->bGenerateOverlapEvents = false;
			}
		}
	}
	GetWorld()->GetTimerManager().SetTimer(ItemNameHandle, this, &AGameItem::HideItemText, ItemTextExpiration, false);
}

void AGameItem::HideItemText()
{
	TextWidget->SetVisibility(false);
}

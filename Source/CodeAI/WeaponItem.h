// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventoryGameItem.h"
#include "WeaponItem.generated.h"

/**
 * 
 */
UCLASS()
class CODEAI_API AWeaponItem : public AInventoryGameItem
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(VisibleAnywhere)
		class UStaticMeshComponent* GunMesh;
	//Sound of the weapon firing
	UPROPERTY(EditDefaultsOnly)
		class USoundBase* GunShotSound;
	//Sound of the weapon out of ammo when attempting to fire
	UPROPERTY(EditDefaultsOnly)
		class USoundBase* OutOfAmmoSound;

	//Amount of ammo per clip
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 ClipSize;
	//Maximum amount of total ammo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MaxAmmo;
	//Current amount of ammo in the clip
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 ClipAmount;
	//Current amount of extra ammo (outside the clip)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 MagsAmount;
	//Distance of effect for holding up enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float HoldUpDistance;

	//Used to determine in which socket to attach the gun mesh to
	UPROPERTY(EditDefaultsOnly)
		bool bIsPistol;

	//Determines whether the weapon draws a 3D Line or not
	uint8 bIsAimed : 1;
	//Determines whether the aiming was cancelled or not
	uint8 bCancelled : 1;

	UFUNCTION()
		virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


public:

	AWeaponItem();
	virtual void Tick(float DeltaSeconds) override;

	void AddAmmo(int32 NewQuantity);
	void DecrementAmmo();
	void Reload();
	FORCEINLINE bool HasAmmo() {return (ClipAmount + MagsAmount > 0);}
	FORCEINLINE bool MagsAreFull() { return (ClipSize + MagsAmount) == MaxAmmo; }
	FORCEINLINE bool AmmoIsFull() {	return (ClipAmount + MagsAmount) == MaxAmmo;}
	FORCEINLINE bool IsPistol() {return bIsPistol;}
	FORCEINLINE bool ClipIsEmpty() {return ClipAmount == 0;}

	UFUNCTION(BlueprintImplementableEvent, Category = Weapon)
		void Equip();
	UFUNCTION(BlueprintImplementableEvent, Category = Weapon)
		void Unequip();

	void AIEquip(class AMyAICharacter* AIChar);

	void UseItemPressed(APawn* Pawn) override;
	void UseItemReleased(APawn* Pawn) override;
	UFUNCTION(BlueprintCallable, category = Weapon)
		void CancelUse();

	UStaticMeshComponent* GetGunMesh() const;
	
};

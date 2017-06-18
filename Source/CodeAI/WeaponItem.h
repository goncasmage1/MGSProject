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
	UPROPERTY(EditAnywhere)
		class USoundBase* GunShotSound;
	//Sound of the weapon firing while silenced
	UPROPERTY(EditAnywhere)
		class USoundBase* GunShotSoundSilenced;
	//Sound of the weapon out of ammo when attempting to fire
	UPROPERTY(EditAnywhere)
		class USoundBase* OutOfAmmoSound;

	AActor* Owner;

	//Amount of ammo per clip
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 ClipSize;
	//Maximum amount of total ammo
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 MaxAmmo;
	//Current amount of ammo in the clip
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 ClipAmount;
	//Current amount of extra ammo (outside the clip)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 MagsAmount;
	//Distance of effect for holding up enemies
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float HoldUpDistance;
	//Damage applied by the weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Damage;
	
	//Used to determine whether this weapon will make any noise when firing
	UPROPERTY(EditAnywhere)
		bool bIsSilenced;
	//Used to determine in which socket to attach the gun mesh to
	UPROPERTY(EditAnywhere)
		bool bIsRifle;
	//Fire rate of the weapon if it's a rifle
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bIsRifle"))
		float FireRate;
	//Timer handle to handle the rifle firing
	FTimerHandle FireRateHandle;

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
	FORCEINLINE bool IsPistol() {return (!bIsRifle);}
	FORCEINLINE bool ClipIsEmpty() {return ClipAmount == 0;}

	void SetOwner(AActor* NewOwner) override;
	FORCEINLINE AActor* GetOwner() { return Owner; }

	void AIEquip(class AMyAICharacter* AIChar);

	void UseItemPressed(bool bShouldHoldUp) override;
	void UseItemReleased(bool bShouldStop) override;
	void RifleShoot();
	UFUNCTION(BlueprintCallable, category = Weapon)
		void CancelUse();

	UStaticMeshComponent* GetGunMesh() const;
	
};

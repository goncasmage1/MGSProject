// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "MyAICharacter.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EAIState : uint8
{
	AI_AttackingTarget 	UMETA(DisplayName = "AttackingTarget"),
	AI_HelpingFriendly 	UMETA(DisplayName = "HelpingFriendly"),
	AI_ChasingTarget 	UMETA(DisplayName = "ChasingTarget"),
	AI_ChasingSound 	UMETA(DisplayName = "ChasingSound"),
	AI_Patrolling		UMETA(DisplayName = "Patrolling"),
	AI_HeldUp			UMETA(DisplayName = "HeldUp"),
	AI_Dead				UMETA(DisplayName = "Dead"),
	AI_None				UMETA(DisplayName = "None")
};

UCLASS()
class CODEAI_API AMyAICharacter : public ACharacter
{
	GENERATED_BODY()

private:

	/******************************************
	*		COMPONENTS
	******************************************/

	//Sound the AI makes when it found a player
	UPROPERTY(EditDefaultsOnly)
		USoundBase* PlayerFoundSound;
	//Sound the AI makes when it hears a sound
	UPROPERTY(EditDefaultsOnly)
		USoundBase* SoundHeardSound;
	//Sound the AI makes when it got hurt
	UPROPERTY(EditDefaultsOnly)
		USoundBase* DamagedSound;
	//Sound the AI makes when it dies
	UPROPERTY(EditDefaultsOnly)
		USoundBase* KilledSound;
	//Sound the AI makes when it is held up
	UPROPERTY(EditDefaultsOnly)
		USoundBase* HeldUpSound;
	//The distance at which even a sound will make the AI know where the player is

	UPROPERTY(EditDefaultsOnly)
		TArray<UAnimationAsset*> DeathAnimations;


	/******************************************
	*		SENSING
	******************************************/

	UPROPERTY(EditAnywhere)
		float MinHearingThresh;
	
	//Indicates whether the player is in sight
	uint8 bCanSeePlayer : 1;
	//Indicates if a noise was heard
	uint8 bHeardNoise : 1;
	//Indicates that the player is visible, but in cover,
	//hence he shouldn't be seen from a certain angle
	uint8 bSightBlockedByCover : 1;

	//Stores a reference to the pawn the AI is chasing
	class ACodeAICharacter* ChasingPawn;
	//Stores the index of the current target point to chase
	uint8 TargetPointNum;

	
	/******************************************
	*		HEALTH
	******************************************/

	float Health;
	UPROPERTY(EditDefaultsOnly)
		float MaxHealth;
	//Time between the moment when the AI attacks the player
	UPROPERTY(EditDefaultsOnly)
		float MinAttackRate;
	//Extra time to add in between attacks
	UPROPERTY(EditDefaultsOnly)
		float MaxAttackRate;
	FTimerHandle DeathHandle;
	
	//Indicates whether the bot is dead
	uint8 bIsDead : 1;


	/******************************************
	*		BEHAVIOR
	******************************************/

	float Time;
	float ExtraTime;

	//Indicates if the AI is being held up
	uint8 bHeldUp : 1;
	//Indicates whether the order to increment the
	//target points by should be reversed
	uint8 bReverseTargetPoints : 1;
	//Indicates whether the AI is pointing a weapon
	uint8 bIsAiming : 1;
	//Indicates whether the player was killed
	uint8 bPlayerDead : 1;
	/*If set to true, the AI will go to the first
	target point when it reached the last one, otherwise
	it will rewing through the target points*/
	UPROPERTY(EditDefaultsOnly)
		bool bResetTargetPoints;

public:

	AMyAICharacter();
	virtual void BeginPlay() override;
	virtual void Tick (float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/******************************************
	*		COMPONENTS
	******************************************/

	UPROPERTY(VisibleAnywhere, Category = "AI")
		class UPawnSensingComponent* PawnSensingComp;
	UPROPERTY(EditAnywhere, Category = "AI")
		class UBehaviorTree* BehaviorTree;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UPawnNoiseEmitterComponent* PawnNoiseEmitterComp;

	
	/******************************************
	*		SENSING
	******************************************/

	UFUNCTION()
		void OnSeePlayer(APawn* Pawn);
	void PlayerSeen();
	UFUNCTION()
		void OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume);
	//Changes the necessary blackboard keys so that the AI bot attacks the player
	void DetectPlayer(class AAnAIController* AICon, APawn* Pawn);
	void SetNoiseHeard(bool bHeard);
	//Checks whether the AI has a line of sight to the player
	bool CheckLineOfSightTo(APawn* pawn);
	//Releases the AI bot from a hold up
	void FreeFromHoldUp();
	/*Plays a sound and reports it to the game*/
	void ReportNoise(USoundBase* SoundToPlay, float Volume);	
	FORCEINLINE bool IsHeldUp() const { return bHeldUp; }


	/******************************************
	*		DAMAGE
	******************************************/

	void OnDeath();
	void OnDestroy();
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser);


	/******************************************
	*		BEHAVIOR
	******************************************/

	//Array of TargetPoints to iterate through
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Target Point", ExposeOnSpawn = true), Category = TargetPoints)
		TArray<class ABotTargetPoint*> TargetPoints;
	//Increments the index of the target points
	void IncrementTargetNum();
	void PlayerKilled();
	FORCEINLINE uint8 GetTargetNum() const { return TargetPointNum; }
	EAIState GetCurrentState() const;

	
	/******************************************
	*		WEAPON
	******************************************/

	bool AddWeapon(class AWeaponItem* WeaponItem);
	void AimWeapon();
	void LowerWeapon();
	void ShootWeapon();
	void SufferHoldUp();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Weapon", ExposeOnSpawn = true), Category = Weapon)
		class AWeaponItem* Weapon;
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
		bool bIsPistolEquipped;
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
		bool bIsRifleEquipped;
	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }
	
};

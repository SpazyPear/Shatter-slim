// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include <Camera/CameraComponent.h>
#include "Revolver.h"
#include "Curves/CurveFloat.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "FMODBlueprintStatics.h"
#include "FMODEvent.h"
#include "BattlePlayerState.h"
#include "FPSCharacter.generated.h"

UCLASS()
class SHATTER_API AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSCharacter(const class FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	UFUNCTION(BlueprintImplementableEvent)
		void RemoveStartWidget();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
		float TeleportFill;

	UCameraComponent* Camera;

	UCameraComponent* TPSCamera;

	FVector CameraDefaultLocation;

	FRotator CameraDefaultRotation;
	
#pragma region Audio

	// FMOD SOUNDS
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* CoinFlipFX;
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* ToggleFX1;
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* ToggleFX2;
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* TeleportFX;
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* TetherFX;
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* ShootFX1;
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* ShootFX2;

#pragma endregion

#pragma region Movement

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float WallJumpForce;

	UPROPERTY(Replicated)
		bool bCanJumpBoost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float JumpBoostForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float CannonBallForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float CannonBallVelocityThreshold;

	void ResetFriction();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float WallJumpAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsSliding;
	
#pragma endregion

#pragma region Shooting
	
	ABattlePlayerState* LastDamagingPlayer;
	
	bool bPullShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TEnumAsByte<EObjectTypeQuery>> ShootObjects;

	UFUNCTION(BlueprintImplementableEvent)
		void EnemyShot();

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateKnockbackIndicator(float KnockbackMultiplier);

	UFUNCTION(Server, Reliable)
		void TakeDamage(float DamageP);

	UPROPERTY(Replicated)
	bool bPushMode;

	UFUNCTION(BlueprintImplementableEvent)
		void ChangeGunModeVisuals(bool bPushShot);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		float MaxKnockbackMultiplier;
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
		TArray<TEnumAsByte<EObjectTypeQuery>> ShootTypes;

	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
		TArray<TEnumAsByte<EObjectTypeQuery>> TeleportTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		float ShotCooldown;
	
	UFUNCTION(BlueprintImplementableEvent)
		void ShootVisuals();

#pragma endregion

#pragma region Tether

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float CameraDirectionFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
		FRuntimeFloatCurve TetherAccelerationCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
		float SwingDamping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
		float SwingClamp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
		float InitialTetherVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
		float TetherMaxDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
		float TetherCurveLength;

	UPROPERTY(EditAnywhere, Category = "Tether")
		float SwingForceConstant;

	UPROPERTY(EditAnywhere, Category = "Tether")
		float SwingForwardBoost;

	UPROPERTY(EditAnywhere, Category = "Tether")
		float SwingControlForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
		TArray<TEnumAsByte<EObjectTypeQuery>> TetherTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
		float PullForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tether")
		float SwingForce;

#pragma endregion

#pragma region Coin Toss

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ADime> DimeBP;

	UPROPERTY(Replicated)
		UStaticMeshComponent* CurrentCoin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		float CoinRefillDelay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		FRuntimeFloatCurve ScrollTossChargeCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		FRuntimeFloatCurve StraightThrowChargeCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		float DownTossForward;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		float DownTossForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		float MaxThrowForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		float MinThrowForce;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		float TossUpForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		float TossForwardForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		float KnockbackFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin Toss")
		float DefaultFriction;

	UFUNCTION(BlueprintImplementableEvent)
		void ShowTether(FVector Target);

	UFUNCTION(BlueprintImplementableEvent)
		void HideTether();
	
#pragma endregion
	
#pragma region Networking
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsGamePaused;

	UFUNCTION(BlueprintCallable, Server, Reliable)
		void LeaveGameServer();

	UFUNCTION(Client, Reliable)
		void DisconnectClient();

	UFUNCTION(Server, Reliable)
		void ToggleClientAuthoratitiveMovement(bool Enabled);

	UFUNCTION(NetMulticast, Reliable)
		void ToggleClientAuthoratitiveMovementMulticast(bool Enabled);

#pragma endregion
	
private:

	USceneComponent* CoinHand;

	ARevolver* Revolver;

	void ResetCanShoot();
	
	const float ClientAuthorityVelocityGate = 7.0;

	int ScrollAmount;

	float StraightTossCharge;

	bool bIsScrolling;

	bool bScrolledThisInterval;

	bool bcanWallSlide;
	
	bool bChargingStraightThow;

	void AttachCoinToHand();
	
	float SwingDistance;
	
	FVector Velocity;
	
	UFUNCTION(Client, Reliable)
		void WaitForCoinSpawn();

	void CheckCoinSpawn();

	UFUNCTION()
		void SettleClientMovement();

	UFUNCTION()
		void HandleCannonballCollision(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	void SetKnockbackFriction();

	FVector PlayerRespawnPos;

	bool bIsGrounded;

	void CoinSpawnTimer();
	
	UFUNCTION(Server, Reliable)
		void CoinSpawn();

	void AcceptScrollUp();

	void AcceptScrollDown();

	void ChargeStraightThrow();
	
	void StraightThrowRelease();

	UFUNCTION(Server, Reliable)
		void TossCoinServer(FVector ThrowForce);

	UFUNCTION(NetMulticast, Reliable)
		void TossCoinMulticast(FVector ThrowForce);
	
	void TossCoin(FVector ThrowForce);

	void TossCoinLocally(FVector ThrowForce);

	void ScrollMonitor();
	
	void Shoot();

	UFUNCTION(Server, Reliable)
		void ShootServer(FHitResult hit);

	void OnShotDime(ADime* Dime);

	void OnShotEnemy(AFPSCharacter* Player);

	void TetherToFrag(FHitResult hit);

	void SetShotModePressed();

	UFUNCTION(Server, Reliable)
	void SetShotMode();

	FHitResult RayCast(ECollisionChannel Channel = ECC_Visibility, float Distance = 100000000);

	void PullToFrag();

	void HandleSwingMovement();

	void Tether();

	void DisconnectTether();

	void MoveRight(float axisValue);

	void MoveForward(float axisValue);

	void Jump();

	void JumpBoost();

	bool WallJump();

	void Slide();

	void SlideReleased();

	void Cannonball();

	void CannonballPushEnemy(AFPSCharacter* Enemy);

	void CannonballTimer();

	void ResetCannonBall();

	bool bcanPush;

	bool bSwitchCamera;

	FVector VectorToPow(FVector vec1, float Pow);

	bool bPulling;

	bool bCanShoot;

	UPrimitiveComponent* PullingFragment;

	FVector LastPullPoint;

	FVector PullDirection;

	float CurrentTetherAcceleration;

	float TimeTethered;

	bool bSwinging;

	void BeginSwing();

	void EndSwing();

	FFMODEventInstance InstanceWrapper;
	FFMODEventInstance InstanceGrapple;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FPSCharacter.h"
#include "BattleCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode {
	CMOVE_None UMETA(DisplayName = "None"),
	CMOVE_Sliding UMETA(DisplayName = "Sliding"),
	CMOVE_MAX UMETA(DisplayName = "Max")
};

UCLASS()
class SHATTER_API UBattleCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Shatter : public FSavedMove_Character
	{
		uint8 Saved_bWantsToSlide : 1;
		public:
			FSavedMove_Shatter();
			typedef FSavedMove_Character Super;
			virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
			virtual void Clear() override;
			virtual uint8 GetCompressedFlags() const override;
			virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
			virtual void PrepMoveFor(ACharacter* Character) override;
	};

	class FNetworkPredictionData_Client_Shatter : public FNetworkPredictionData_Client_Character
	{
		public:
			FNetworkPredictionData_Client_Shatter(const UCharacterMovementComponent& ClientMovement);
			typedef FNetworkPredictionData_Client_Character Super;
			virtual FSavedMovePtr AllocateNewMove() override;
	};
	
	public:

		UPROPERTY(EditDefaultsOnly, Category = "Slide")
			float SlideMinWalkSpeedRatio = 0.7;

		float SlideMinSpeed;

		UPROPERTY(EditDefaultsOnly, Category = "Slide")
			float SlideEnterImpulse = 500;

		UPROPERTY(EditDefaultsOnly, Category = "Slide")
			float SlideGravityForce = 5000;

		UPROPERTY(EditDefaultsOnly, Category = "Slide")
			float SlideFriction = 1.3;

		UFUNCTION(BlueprintPure) 
			bool IsCustomMovementMode(ECustomMovementMode Mode) const;

		void Slide();

		void SlideReleased();

		virtual bool IsMovingOnGround() const override;

		virtual bool CanCrouchInCurrentState() const override;

		virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

		virtual void UpdateFromCompressedFlags(uint8 Flags) override;

		virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	protected:
		virtual void InitializeComponent() override;
		virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
		virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
		virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
		
	private:
		UCapsuleComponent* Capsule;
		float CapsuleHeight;
		AFPSCharacter* CharacterOwner;
		UPROPERTY(Replicated)
		bool Safe_bWantsToSlide;
		bool bWantsToSlide;
		bool bSlideTriggered;
		bool CanSlide();
		void EnterSlide();
		void ExitSlide();
		void PhysSlide(float DeltaTime, int32 Iterations);
		bool GetSlideSurface(FHitResult& Hit) const;
};

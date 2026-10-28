// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include <Net/UnrealNetwork.h>

#define printOnScreen(x) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT(x));	
#define print(x) if (CharacterOwner->IsLocallyControlled()) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, x);	
#define FLAG_WantsToSlide FSavedMove_Character::FLAG_Custom_0

bool UBattleCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode Mode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == Mode;
}

void UBattleCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	SlideMinSpeed = MaxWalkSpeed * SlideMinWalkSpeedRatio;
	CharacterOwner = Cast<AFPSCharacter>(GetCharacterOwner());
	Capsule = Cast<UCapsuleComponent>(GetOwner()->GetRootComponent());
}

void UBattleCharacterMovementComponent::Slide()
{
	Safe_bWantsToSlide = true;
}

void UBattleCharacterMovementComponent::SlideReleased()
{
	Safe_bWantsToSlide = false;
}

void UBattleCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UBattleCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	if (MovementMode == MOVE_Walking && Safe_bWantsToSlide)
	{
		if (CanSlide())
		{
			EnterSlide();
		}
	}

	if (IsCustomMovementMode(CMOVE_Sliding) && !Safe_bWantsToSlide)
	{
		//printString("tick exit")
		ExitSlide();
	}
}

void UBattleCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	Super::PhysCustom(DeltaTime, Iterations);

	switch (CustomMovementMode) 
	{
		case CMOVE_Sliding:
			PhysSlide(DeltaTime, Iterations);
			break;
		default:
			UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	}
}

bool UBattleCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Sliding);
}

bool UBattleCharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

void UBattleCharacterMovementComponent::EnterSlide()
{
	Velocity += Velocity.GetSafeNormal2D() * SlideEnterImpulse;
	Capsule->SetCapsuleHalfHeight(CapsuleHeight / 2.0);
	SetMovementMode(MOVE_Custom, CMOVE_Sliding);
}

void UBattleCharacterMovementComponent::ExitSlide()
{
	Safe_bWantsToSlide = false;
	Capsule->SetCapsuleHalfHeight(CapsuleHeight);
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Hit);
	SetMovementMode(MOVE_Walking);
}

void UBattleCharacterMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME) return;
	
	RestorePreAdditiveRootMotionVelocity();
	
	if (!CanSlide()) 
	{
		ExitSlide();
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	// Apply gravity
	Velocity += SlideGravityForce * FVector::DownVector * DeltaTime;

	// Strafe
	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > 0.5) 
	{
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	}
	else
	{
		Acceleration = FVector::ZeroVector;
	}

	FHitResult SurfaceHit;
	GetSlideSurface(SurfaceHit);

	// Calc Velocity
	bool bHasAnimRootMotion = HasAnimRootMotion() && CurrentRootMotion.HasOverrideVelocity();
	if (!bHasAnimRootMotion) 
	{
		CalcVelocity(DeltaTime, SlideFriction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(DeltaTime);

	// Perform Move
	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentQuat();
	FHitResult Hit(1);
	FVector Adjusted = Velocity * DeltaTime;
	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal).GetSafeNormal();
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, SurfaceHit.Normal).ToQuat();
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);
	
	if (Hit.Time < 1.0f) 
	{
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.0f - Hit.Time), Hit.Normal, Hit, true);
	}

	if (!CanSlide())
	{
		//printString("after impact")
		ExitSlide();
	}
	
	if (!bJustTeleported && !bHasAnimRootMotion)
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
	}
}

bool UBattleCharacterMovementComponent::CanSlide()
{
	FHitResult Hit;
	return GetSlideSurface(Hit) && Velocity.SizeSquared() > FMath::Pow(SlideMinSpeed, 2);
}

bool UBattleCharacterMovementComponent::GetSlideSurface(FHitResult& Hit) const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CharacterOwner);
	Params.AddIgnoredActors(CharacterOwner->Children);
	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, Params);
}

void UBattleCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UBattleCharacterMovementComponent, Safe_bWantsToSlide, COND_SimulatedOnly);
}

#pragma region Networking

void UBattleCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	Safe_bWantsToSlide = (Flags && FLAG_WantsToSlide) != 0;
}

FNetworkPredictionData_Client* UBattleCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)
	if (ClientPredictionData == nullptr)
	{
		UBattleCharacterMovementComponent* MutableThis = const_cast<UBattleCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Shatter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140;
	}
	return ClientPredictionData;
}

UBattleCharacterMovementComponent::FSavedMove_Shatter::FSavedMove_Shatter()
{
	Saved_bWantsToSlide = 0;
}

bool UBattleCharacterMovementComponent::FSavedMove_Shatter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_Shatter* NewShatterMove = static_cast<FSavedMove_Shatter*>(NewMove.Get());
	if (Saved_bWantsToSlide != NewShatterMove->Saved_bWantsToSlide) return false;
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UBattleCharacterMovementComponent::FSavedMove_Shatter::Clear()
{
	Super::Clear();
	Saved_bWantsToSlide = 0;
}

uint8 UBattleCharacterMovementComponent::FSavedMove_Shatter::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();
	if (Saved_bWantsToSlide) Result |= FLAG_WantsToSlide;
	return Result;
}

void UBattleCharacterMovementComponent::FSavedMove_Shatter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	const UBattleCharacterMovementComponent* CharacterMovement = Cast<UBattleCharacterMovementComponent>(Character->GetCharacterMovement());
	Saved_bWantsToSlide = CharacterMovement->Safe_bWantsToSlide;
}

void UBattleCharacterMovementComponent::FSavedMove_Shatter::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	UBattleCharacterMovementComponent* CharacterMovement = Cast<UBattleCharacterMovementComponent>(Character->GetCharacterMovement());
	CharacterMovement->Safe_bWantsToSlide = Saved_bWantsToSlide;
}

UBattleCharacterMovementComponent::FNetworkPredictionData_Client_Shatter::FNetworkPredictionData_Client_Shatter(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr UBattleCharacterMovementComponent::FNetworkPredictionData_Client_Shatter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Shatter());
}

#pragma endregion

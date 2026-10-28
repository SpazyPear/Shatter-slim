// Fill out your copyright notice in the Description page of Project Settings.

#define FragmentChannel ECC_GameTraceChannel3
#define WallObjectType ETraceTypeQuery::TraceTypeQuery1
#define Move_Sliding EMovementMode::Move_Custom

#include "FPSCharacter.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"
#include "Dime.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetStringLibrary.h"
#include <Components/BoxComponent.h>
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include <Net/UnrealNetwork.h>
#include "BattleGameModeBase.h"
#include "EOSGameInstance.h"
#include "FMODBlueprintStatics.h"
#include "FMODEvent.h"
#include "ShatterPlayerController.h"
#include "BattleCharacterMovementComponent.h"



AFPSCharacter::AFPSCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UBattleCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bPushMode = true;
	bCanShoot = true;
	bcanPush = false;
	TPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPSCamera"));
	TPSCamera->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	IsGamePaused = false;
	CoinHand = Cast<USceneComponent>(GetDefaultSubobjectByName(FName("CoinHand")));
	Camera = Cast<UCameraComponent>(GetDefaultSubobjectByName(FName("FirstPersonCamera")));
	Revolver = Cast<ARevolver>(Cast<UChildActorComponent>(GetDefaultSubobjectByName(FName("Revolver")))->GetChildActor());
	TPSCamera->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator); //-330, 0, 270 FRotator(-30, 0, 0)
	TPSCamera->SetActive(false);

	Cast<UCapsuleComponent>(GetRootComponent())->OnComponentHit.AddDynamic(this, &AFPSCharacter::HandleCannonballCollision);
	bIsSliding = false;
	//WallJumpIncrement = 0;

	TActorIterator<APlayerStart> PlayerStart = TActorIterator<APlayerStart>(GetWorld());
	PlayerRespawnPos = PlayerStart->GetActorLocation();
	
	
	/*if (HasAuthority())
	{
		CoinSpawnTimer();
	}*/
	
	if (IsLocallyControlled())
	{
		Revolver->Camera = Camera;
		Revolver->SetOwner(this);
		CoinSpawnTimer();
		
	}

	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &AFPSCharacter::ScrollMonitor, 0.1, true, 0);

	CameraDefaultLocation = FVector(-39.560001, 0, 64.248421);
	CameraDefaultRotation = FRotator::ZeroRotator;
}

// Called every frame
void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PullToFrag();
	HandleSwingMovement();

	if (GetCharacterMovement()->IsMovingOnGround() && bCanJumpBoost && !bPulling) 
	{
		bCanJumpBoost = false;		
	}

	if (bChargingStraightThow) StraightTossCharge += DeltaTime;
}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Left Click", IE_Pressed, this, &AFPSCharacter::Shoot);
	PlayerInputComponent->BindAction("Pull / Push Toggle", IE_Pressed, this, &AFPSCharacter::SetShotModePressed);
	PlayerInputComponent->BindAction("Right Click", IE_Pressed, this, &AFPSCharacter::Tether);
	PlayerInputComponent->BindAction("Right Click", IE_Released, this, &AFPSCharacter::DisconnectTether);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFPSCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	PlayerInputComponent->BindAction("Swing", IE_Pressed, this, &AFPSCharacter::BeginSwing);
	PlayerInputComponent->BindAction("Swing", IE_Released, this, &AFPSCharacter::EndSwing);

	PlayerInputComponent->BindAction("Cannonball", IE_Pressed, this, &AFPSCharacter::Cannonball);
	PlayerInputComponent->BindAction("Slide", IE_Pressed, this, &AFPSCharacter::Slide);
	PlayerInputComponent->BindAction("Slide", IE_Released, this, &AFPSCharacter::SlideReleased);

	PlayerInputComponent->BindAction("Straight Throw", IE_Pressed, this, &AFPSCharacter::ChargeStraightThrow);
	PlayerInputComponent->BindAction("Straight Throw", IE_Released, this, &AFPSCharacter::StraightThrowRelease);
	PlayerInputComponent->BindAction("Mouse Wheel Up", IE_Pressed, this, &AFPSCharacter::AcceptScrollUp);
	PlayerInputComponent->BindAction("Mouse Wheel Down", IE_Pressed, this, &AFPSCharacter::AcceptScrollDown);

	PlayerInputComponent->BindAxis("Move Right / Left", this, &AFPSCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AFPSCharacter::MoveForward);
}

void AFPSCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	//Super::FellOutOfWorld(dmgType);
	Cast<AShatterPlayerController>(GetController())->OnPlayerDie.Broadcast(this, GetActorLocation());
}

void AFPSCharacter::HandleCannonballCollision(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bcanPush) 
	{
		if (AFPSCharacter* Enemy = Cast<AFPSCharacter>(OtherActor))
		{
			if (GetCharacterMovement()->Velocity.Length()) 
			{
				CannonballPushEnemy(Enemy);
			}
		}
	}
}

void AFPSCharacter::MoveRight(float axisValue)
{
	if (!IsGamePaused) 
	{
		if (!bSwinging)
			GetCharacterMovement()->AddInputVector(GetActorRightVector() * axisValue);
		else
			GetCharacterMovement()->AddForce(GetActorRightVector() * axisValue * SwingControlForce);
	}
}

void AFPSCharacter::MoveForward(float axisValue)
{
	if (!IsGamePaused) 
	{
		if (!bSwinging)
			GetCharacterMovement()->AddInputVector(GetActorForwardVector() * axisValue);
		else
			GetCharacterMovement()->AddForce(GetActorForwardVector() * axisValue * SwingControlForce);
	}
}

#pragma region Coin

void AFPSCharacter::ResetFriction()
{
	GetCharacterMovement()->GroundFriction = DefaultFriction;
}

void AFPSCharacter::CoinSpawnTimer()
{
	FTimerHandle coinTimer;
	FTimerDelegate coinDelegate;

	GetWorldTimerManager().SetTimer(coinTimer, this, &AFPSCharacter::CoinSpawn, 2.0, false, 2.0);
}

// Lower the players ground friction so that when they are knockedback horizontally, they slide away
void AFPSCharacter::SetKnockbackFriction()
{
	GetCharacterMovement()->GroundFriction = KnockbackFriction;
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &AFPSCharacter::ResetFriction, 2, false, 2);
}

void AFPSCharacter::CoinSpawn_Implementation()
{
	ADime* Coin = GetWorld()->SpawnActor<ADime>(DimeBP, CoinHand->GetComponentLocation(), CoinHand->GetComponentRotation());
	Coin->AttachToComponent(CoinHand, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Coin->HoldingPlayer = this;
	CurrentCoin = Cast<UStaticMeshComponent>(Coin->GetRootComponent());
}

void AFPSCharacter::AttachCoinToHand()
{
	CurrentCoin->GetOwner()->SetActorRelativeLocation(FVector::ZeroVector);
	CurrentCoin->GetOwner()->SetActorRelativeRotation(FRotator::ZeroRotator);
}

void AFPSCharacter::WaitForCoinSpawn_Implementation()
{
	CheckCoinSpawn();
}

void AFPSCharacter::CheckCoinSpawn()
{
	if (CurrentCoin == nullptr)
	{
		FTimerHandle NullCheckHandle;
		GetWorldTimerManager().SetTimer(NullCheckHandle, this, &AFPSCharacter::CheckCoinSpawn, 0.1, false, 0.1);
	}
	else
	{
		AttachCoinToHand();
	}
}

void AFPSCharacter::AcceptScrollUp()
{
	ScrollAmount++;
	bScrolledThisInterval = true;
}

void AFPSCharacter::AcceptScrollDown()
{
	ScrollAmount--;
	bScrolledThisInterval = true;
}

void AFPSCharacter::ChargeStraightThrow()
{
	bChargingStraightThow = true;
}

void AFPSCharacter::StraightThrowRelease()
{
	bChargingStraightThow = false;
	FVector Forward = Camera->GetForwardVector();
	float Multiplier = StraightThrowChargeCurve.GetRichCurve()->Eval(StraightTossCharge);
	TossCoin(Forward * Multiplier);
	StraightTossCharge = 0;
}

void AFPSCharacter::ScrollMonitor() 
{
	if (bIsScrolling && !bScrolledThisInterval)
	{
		bIsScrolling = false;
		float Force = ScrollTossChargeCurve.GetRichCurve()->Eval(ScrollAmount);
		FVector Forward = Camera->GetForwardVector();
		if (ScrollAmount >= 0) TossCoin(FVector(Forward.X * TossForwardForce * Force, Forward.Y * TossForwardForce * Force, TossUpForce * Force));
		else TossCoin(FVector(Forward.X * DownTossForward, Forward.Y * DownTossForward, -DownTossForce));
		ScrollAmount = 0;
	}
	else if (!bIsScrolling && bScrolledThisInterval)
	{
		bIsScrolling = true;
	}
	bScrolledThisInterval = false;
}

void AFPSCharacter::TossCoinServer_Implementation(FVector ThrowForce)
{
	if (CurrentCoin)
	{
		Cast<ADime>(CurrentCoin->GetOwner())->OnTossed();
		TossCoinMulticast(ThrowForce);
		CurrentCoin = nullptr;
		CoinSpawnTimer();
	}
}

void AFPSCharacter::TossCoinMulticast_Implementation(FVector ThrowForce)
{
	if (!IsLocallyControlled())
		TossCoinLocally(ThrowForce);
}

void AFPSCharacter::TossCoinLocally(FVector ThrowForce)
{
	if (CurrentCoin && !ThrowForce.Equals(FVector::ZeroVector))
	{
		CurrentCoin->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
		CurrentCoin->SetSimulatePhysics(true);
		CurrentCoin->AddImpulse(ThrowForce);
	}
}

void AFPSCharacter::TossCoin(FVector ThrowForce)
{
	TossCoinLocally(ThrowForce);
	TossCoinServer(ThrowForce);
}


#pragma endregion

#pragma region Jumping

void AFPSCharacter::Jump()
{
	if (WallJump()) return;
	
	if (bCanJumpBoost) { JumpBoost(); }

	else { ACharacter::Jump(); }
	
}

void AFPSCharacter::JumpBoost()
{
	bCanJumpBoost = false;
	LaunchCharacter(JumpBoostForce * FVector::UpVector, false, false);
}

bool AFPSCharacter::WallJump()
{
	if (GetCharacterMovement()->IsFalling())
	{
		FHitResult hit = RayCast(ECC_Visibility, 420);

		if (hit.bBlockingHit) 
		{
			FVector Direction = (GetActorLocation() - hit.ImpactPoint).RotateAngleAxis(WallJumpAngle, GetActorRightVector());
			Direction.Normalize();
			LaunchCharacter(WallJumpForce * Direction, true, true);			
			return true;
		}
		
	}
	return false;
}

#pragma endregion

#pragma region Sliding

void AFPSCharacter::Slide()
{
	Cast<UBattleCharacterMovementComponent>(GetCharacterMovement())->Slide();
}

void AFPSCharacter::SlideReleased()
{
	Cast<UBattleCharacterMovementComponent>(GetCharacterMovement())->SlideReleased();
}

#pragma endregion

#pragma region Cannonball

void AFPSCharacter::Cannonball()
{
	if (GetCharacterMovement()->Velocity.Size() > 1300.0f)
	{
		Camera->SetActive(false);
		TPSCamera->SetActive(true);
		bcanPush = true;
		GetCharacterMovement()->MaxWalkSpeed += 600;
		CannonballTimer();
	}
}

void AFPSCharacter::CannonballPushEnemy(AFPSCharacter* Enemy)
{
	Enemy->SetKnockbackFriction();
	FVector enemyPushDirection = (GetActorLocation() - Enemy->GetActorLocation());
	enemyPushDirection.Normalize();
	float enemyPushForce = FVector::DotProduct(enemyPushDirection, GetActorForwardVector()) * CannonBallForce;
	Enemy->LaunchCharacter(enemyPushDirection * CannonBallForce * GetCharacterMovement()->Velocity, false, false);
}

void AFPSCharacter::CannonballTimer()
{
	FTimerHandle cameraTimer;
	GetWorldTimerManager().SetTimer(cameraTimer, this, &AFPSCharacter::ResetCannonBall, 1.0f, false);
}

void AFPSCharacter::ResetCannonBall()
{
	GetCharacterMovement()->GroundFriction = DefaultFriction;
	 Camera->SetActive(true);
	 bcanPush = false;
	 bSwitchCamera = true;
	 GetCharacterMovement()->MaxWalkSpeed -= 600;
	 TPSCamera->SetActive(false);
}

#pragma endregion

#pragma region Grapple

void AFPSCharacter::PullToFrag()
{

	if (bPulling && TimeTethered < TetherMaxDuration)
	{
		//TimeTethered += GetWorld()->GetDeltaSeconds();
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		FVector Direction = (PullingFragment->GetOwner()->GetActorLocation() - GetActorLocation());
		float DistanceFactor = 1; //FVector::Distance(GetActorLocation(), PullingFragment->GetComponentLocation());
		Direction.Normalize();
		Direction *= PullForce * DistanceFactor;
		FVector CameraDirection = Camera->GetForwardVector() * CameraDirectionFactor;
		//CurrentTetherAcceleration = TetherAccelerationCurve.EditorCurveData.Eval(TimeTethered / TetherCurveLength);
		Velocity = (Direction + CameraDirection);
		//GetCharacterMovement()->Velocity = Velocity;
		GetCharacterMovement()->AddForce(Direction);
	}
	else if (TimeTethered > TetherMaxDuration)
		DisconnectTether();
}

void AFPSCharacter::Tether()
{
	FHitResult hit = RayCast();
	if (hit.bBlockingHit)
	{
		TetherToFrag(hit);
	}
}

void AFPSCharacter::TetherToFrag(FHitResult hit)
{
	// Grapple toward the hit coin fragment
	ToggleClientAuthoratitiveMovement(true);
	LastPullPoint = hit.ImpactPoint;
	PullingFragment = Cast<UPrimitiveComponent>(hit.GetActor()->GetRootComponent());
	if (PullingFragment && PullingFragment->GetCollisionObjectType() == FragmentChannel)
	{
		ShowTether(PullingFragment->GetComponentLocation());
		PullingFragment->SetSimulatePhysics(false);
		bPulling = true;
		Velocity = (PullingFragment->GetOwner()->GetActorLocation() - GetActorLocation());
		Velocity.Normalize();
		Velocity *= InitialTetherVelocity;
		InstanceGrapple = UFMODBlueprintStatics::PlayEventAtLocation(this, TetherFX, GetTransform(), true);
	}
}

void AFPSCharacter::DisconnectTether()
{
	if (bPulling)
	{
		HideTether();
		TimeTethered = 0;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		Velocity = FVector(0, 0, 0);
		bPulling = false;
		CurrentTetherAcceleration = 0;
	}
	if (PullingFragment != nullptr)
	{
		PullingFragment->SetSimulatePhysics(true);
	}
	bCanJumpBoost = true;
	UFMODBlueprintStatics::EventInstanceSetParameter(InstanceGrapple, "END", 1.0); // Stop Grapple loop sound
	EndSwing();
	SettleClientMovement();
}

void AFPSCharacter::HandleSwingMovement()
{
	if (bSwinging)
	{

		// Swing movement logic that orbits the player around the swing point with an adjustable amount of speed.

		/*float ScaleDegree = FVector::DotProduct(GetCharacterMovement()->Velocity, GetActorLocation() - PullingFragment->GetComponentLocation());
		FVector Direction = GetActorLocation() - PullingFragment->GetComponentLocation();
		Direction.Normalize();
		
		GetCharacterMovement()->AddForce(Direction * ScaleDegree * SwingForceConstant);

		FVector ForceDirection = FVector::CrossProduct(FVector::RightVector, Direction);
		GetCharacterMovement()->AddForce(-ForceDirection * SwingForwardBoost);*/

		//This is swing movement thats more player controlled, and is feels more like youre hanging on a rope. Decided to use the above one because I like how the momentum is more orbital.
		
		float ScaleDegree = FVector::Distance(GetActorLocation(), PullingFragment->GetComponentLocation());

		ScaleDegree -= (SwingDistance * 0.8f);
		FVector Direction = PullingFragment->GetComponentLocation() - GetActorLocation();
		Direction.Normalize();
		GetCharacterMovement()->AddForce(Direction * FMath::Clamp((ScaleDegree * SwingForceConstant), 0, SwingClamp));
		//GetCharacterMovement()->AddForce(GetRootComponent()->GetForwardVector() * SwingForwardBoost);
		GetCharacterMovement()->AddForce(-Direction * SwingForwardBoost);

	}
}

void AFPSCharacter::BeginSwing()
{
	if (bPulling) 
	{
		bPulling = false;
		bSwinging = true;
		SwingDistance = FVector::Distance(GetActorLocation(), PullingFragment->GetComponentLocation());
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	}
}

void AFPSCharacter::EndSwing()
{
	if (bSwinging) 
	{
		HideTether();
		bSwinging = false;
	}
}

#pragma endregion

#pragma region Shooting

void AFPSCharacter::ResetCanShoot()
{
	bCanShoot = true;
}

void AFPSCharacter::Shoot()
{
	if (bCanShoot)
	{
		InstanceWrapper = UFMODBlueprintStatics::PlayEventAtLocation(this, FMath::RandBool() ? ShootFX1 : ShootFX2, GetTransform(), true);
		bCanShoot = false;
		FTimerHandle handle;
		GetWorldTimerManager().SetTimer(handle, this, &AFPSCharacter::ResetCanShoot, ShotCooldown, false, -1);

		ShootVisuals();

		FHitResult hit = RayCast();
		if (hit.bBlockingHit)
		{
			ShootServer(hit);
		}
	}
}

void AFPSCharacter::TakeDamage_Implementation(float DamageP)
{
	APlayerController* C = Cast<APlayerController>(GetController());
	if (C->GetPlayerState<ABattlePlayerState>()->KnockbackMultiplier + DamageP < MaxKnockbackMultiplier) {
		C->GetPlayerState<ABattlePlayerState>()->KnockbackMultiplier += DamageP;
		UpdateKnockbackIndicator(C->GetPlayerState<ABattlePlayerState>()->KnockbackMultiplier);
	}
}

// Swap between push and pull mode
void AFPSCharacter::SetShotModePressed()
{
	// Play sound based on which mode swapping to
	InstanceWrapper = UFMODBlueprintStatics::PlayEventAtLocation(this, bPushMode ? ToggleFX1 : ToggleFX2, GetTransform(), true);
	Revolver->SetShotMode();
	SetShotMode();
}

void AFPSCharacter::SetShotMode_Implementation()
{
	bPushMode = !bPushMode;
}

FHitResult AFPSCharacter::RayCast(ECollisionChannel Channel, float Distance)
{
	FHitResult Hit;
	FVector start = Camera->GetComponentLocation();
	FVector end = Camera->GetComponentLocation() + (Camera->GetForwardVector() * Distance);
	FCollisionQueryParams para;
	para.AddIgnoredActor(this);
	if (CurrentCoin) {
		para.AddIgnoredActor(CurrentCoin->GetOwner());
	}

	GetWorld()->LineTraceSingleByChannel(Hit, start, end, Channel, para);
	return Hit;
}

void AFPSCharacter::ShootServer_Implementation(FHitResult hit)
{		
	if (ADime* Dime = Cast<ADime>(hit.GetActor()))
	{
		OnShotDime(Dime);
	}
	else if (AFPSCharacter* Enemy = Cast<AFPSCharacter>(hit.GetActor()))
	{
		OnShotEnemy(Enemy);
	}
}


void AFPSCharacter::OnShotDime(ADime* Dime)
{
	if (Dime->HoldingPlayer)
	{
		Dime->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
		Dime->HoldingPlayer->CoinSpawnTimer();
		Dime->HoldingPlayer->CurrentCoin = nullptr;
	}
	Dime->SetPolarity(bPushMode);
	Dime->Shatter(Camera->GetForwardVector(), this);
}

void AFPSCharacter::OnShotEnemy(AFPSCharacter* Player)
{
	TeleportFill = FMath::Clamp(TeleportFill + 0.2f, 0, 1);
	EnemyShot();
	Player->TakeDamage(Damage);
}

#pragma endregion

#pragma region Networking

void AFPSCharacter::ToggleClientAuthoratitiveMovement_Implementation(bool Enabled)
{
	ToggleClientAuthoratitiveMovementMulticast(Enabled);
}

void AFPSCharacter::ToggleClientAuthoratitiveMovementMulticast_Implementation(bool Enabled)
{
	GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = Enabled;
	GetCharacterMovement()->bServerAcceptClientAuthoritativePosition = Enabled;
}

void AFPSCharacter::SettleClientMovement()
{
	FTimerHandle TimerHandle;
	FTimerDelegate Delegate;
	if (GetCharacterMovement()->Velocity.Length() > ClientAuthorityVelocityGate)
	{
		Delegate.BindUFunction(this, FName("SettleClientMovement"));
		GetWorldTimerManager().SetTimer(TimerHandle, Delegate, 0.1, false);
		return;
	}
	else
	{
		ToggleClientAuthoratitiveMovement(false);
	}
}

void AFPSCharacter::DisconnectClient_Implementation()
{
	ENetRole role = GetLocalRole();
	GetWorld()->GetFirstPlayerController()->ClientReturnToMainMenuWithTextReason(FText::FromString("You left the game"));
	Cast<UEOSGameInstance>(GetWorld()->GetFirstPlayerController()->GetGameInstance())->LeaveGame();
}

void AFPSCharacter::LeaveGameServer_Implementation()
{
	for (auto it = TActorIterator<AFPSCharacter>(GetWorld()); it; ++it) 
	{
		it->DisconnectClient();
	}
}

void AFPSCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSCharacter, TeleportFill);
	DOREPLIFETIME(AFPSCharacter, CurrentCoin);
	DOREPLIFETIME(AFPSCharacter, bCanJumpBoost);
	DOREPLIFETIME(AFPSCharacter, bPushMode);
}

#pragma endregion

FVector AFPSCharacter::VectorToPow(FVector vec1, float Pow)
{
	return FVector(FMath::Pow(vec1.X, Pow), FMath::Pow(vec1.Y, Pow), FMath::Pow(vec1.Z, Pow));
}
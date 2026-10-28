// Fill out your copyright notice in the Description page of Project Settings.


#include "Dime.h"
#include "Fragment.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FPSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BattlePlayerState.h"
#include "TimerManager.h"
#include "FMODBlueprintStatics.h"
#include "FMODEvent.h"

// Sets default values
ADime::ADime()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	FragCount = 5;
	//bHeld = true;
	bPush = true;
	
}

// Called when the game starts or when spawned
void ADime::BeginPlay()
{
	Super::BeginPlay();
	Hitbox = Cast<UBoxComponent>(GetDefaultSubobjectByName(FName("Box")));
	SetActorRelativeLocation(FVector::ZeroVector);
	SetActorRelativeRotation(FRotator::ZeroRotator);
}

// Called every frame
void ADime::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Shatter the coin into fragments
void ADime::Shatter(FVector HitVector, AFPSCharacter* PlayerThatShot)
{
	for (int x = 0; x < FragCount; x++) {
		float angle = (x + (FragCount / 2.0)) * 20.0;
		FVector axis = FVector::CrossProduct(HitVector, FVector(0, 0, 1));
		FVector plane = HitVector.RotateAngleAxis(angle, axis).RotateAngleAxis(90, FVector(0, 0, 1));
		FVector launchVector = (plane + (HitVector * 0.7)) * SplitForce;
		AFragment* frag = GetWorld()->SpawnActor<AFragment>(FragmentBP, GetRootComponent()->GetComponentLocation(), GetRootComponent()->GetComponentRotation());
		UStaticMeshComponent* prim = Cast<UStaticMeshComponent>(frag->GetRootComponent());
		prim->AddImpulse(launchVector);
	}
	FireImpulse(PlayerThatShot);
	InstanceWrapper = UFMODBlueprintStatics::PlayEventAtLocation(this, ShatterFX, GetTransform(), true);
	UFMODBlueprintStatics::EventInstanceSetParameter(InstanceWoosh, "END", 1.0); // Kill woosh when coin is shot
	Destroy();
}


void ADime::SetPolarity(bool bisPush)
{
	this->bPush = bisPush;
}

// Apply impulse to the players in range of the explosion
void ADime::FireImpulse(AFPSCharacter* PlayerThatShot)
{
	TArray<FHitResult> OutHits;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), GetActorLocation(), GetActorLocation(), ImpulseRadius, ImpulseObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, OutHits, true, FLinearColor::Red, FLinearColor::Green, 4.0);
	for (const FHitResult& hit : OutHits) {
		AFPSCharacter* Player = Cast<AFPSCharacter>(hit.GetActor());
		if (Player) 
		{
			FVector Direction = hit.GetActor()->GetActorLocation() - GetActorLocation();
			Direction.Normalize();
			float Distance = FVector::Dist(hit.GetActor()->GetActorLocation(), GetActorLocation());
			float ImpulseStrength;

			if (Player == PlayerThatShot)
			{
				// Apply a different impulse to the player that shot the coin
				ImpulseStrength = bPush ? SelfPushStrength : SelfPullStrength;
				ImpulseStrength *= SelfImpulseFalloff.GetRichCurve()->Eval(FMath::GetMappedRangeValueClamped(FVector2D(0, SelfImpulseRadius), FVector2D(0, 1), Distance));
			}
			else
			{
				ImpulseStrength = bPush ? PushStrength : PullStrength;
				ImpulseStrength *= ImpulseFalloff.GetRichCurve()->Eval(FMath::GetMappedRangeValueClamped(FVector2D(0, ImpulseRadius), FVector2D(0, 1), Distance));
				Player->LastDamagingPlayer = PlayerThatShot->GetPlayerState<ABattlePlayerState>();
			}
			FTimerHandle handle;
			Player->GetCharacterMovement()->GroundFriction = Player->KnockbackFriction;
			GetWorldTimerManager().SetTimer(handle, Player, &AFPSCharacter::ResetFriction, SlipDuration, false, SlipDuration);
			Player->LaunchCharacter(Direction * ImpulseStrength * Player->GetPlayerState<ABattlePlayerState>()->KnockbackMultiplier, false, false);
		}
	}
}

void ADime::OnTossed_Implementation()
{
	StaggerReplication();
	HoldingPlayer = nullptr;
}

void ADime::StaggerReplication()
{
	SetReplicates(false);
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &ADime::EnableReplication, 3, false, 3);
}

void ADime::EnableReplication()
{
	SetReplicates(true);
}




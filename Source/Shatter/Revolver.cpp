// Fill out your copyright notice in the Description page of Project Settings.


#include "Revolver.h"
#include "FPSCharacter.h"


// Sets default values
ARevolver::ARevolver()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bPushMode = true;

}

// Called when the game starts or when spawned
void ARevolver::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARevolver::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Not currently being used due to issues with replication in this class. TODO move shoot logic to here and fix replication
void ARevolver::Shoot()
{
	UE_LOG(LogTemp, Warning, TEXT("%d shoot role %s"), GetLocalRole(), *(GetName()))

	if (GetOwner())
		UE_LOG(LogTemp, Warning, TEXT("Has owner"))

	FVector start = Camera->GetComponentLocation();
	FVector end = Camera->GetComponentLocation() + (Camera->GetForwardVector() * 1000000.0);
	FHitResult hit;
	
	GetWorld()->LineTraceSingleByObjectType(hit, start, end, ShootObjects);
	ADime* dime = Cast<ADime>(hit.GetActor());
	AFPSCharacter* enemy = Cast<AFPSCharacter>(hit.GetActor());
	if (hit.bBlockingHit && dime) 
	{
		dime->SetPolarity(bPushMode);
		ShatterServer(dime);
	}
	else if (hit.bBlockingHit && enemy) 
	{
		enemy->TakeDamage(Damage);
	}
}

void ARevolver::ShatterServer_Implementation(ADime* dime)
{
	//dime->Shatter(Camera->GetForwardVector());
}

void ARevolver::SetShotMode_Implementation()
{
	bPushMode = !bPushMode;
	ChangeGunModeVisuals(!bPushMode);
}


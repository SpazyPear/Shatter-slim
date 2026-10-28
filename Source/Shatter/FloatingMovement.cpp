// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingMovement.h"

// Sets default values for this component's properties
UFloatingMovement::UFloatingMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Offset = FMath::FRandRange(0.0, 1.0);
	Progress += Offset;
	// ...
}


// Called when the game starts
void UFloatingMovement::BeginPlay()
{
	Super::BeginPlay();

	Actor = GetOwner();
	StartZ = Actor->GetActorLocation().Z;
}


// For future use to make buildings float and move up and down
void UFloatingMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector PrevLocation = Actor->GetActorLocation();
	Actor->SetActorLocation(FVector(PrevLocation.X, PrevLocation.Y, StartZ + (Curve.GetRichCurve()->Eval(Progress) * Scale)));
	Progress = Progress > 1 ? 0 : Progress + DeltaTime * Rate;
}


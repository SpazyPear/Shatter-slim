// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerState.h"
#include "FPSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "ShatterPlayerController.h"
#include "FPSCharacter.h"


ABattlePlayerState::ABattlePlayerState()
{
	KnockbackMultiplier = 1;
}

void ABattlePlayerState::BeginPlay()
{
	bReplicates = true;
}

void ABattlePlayerState::OnPossess(APawn* Pawn)
{
	PlayerRef = Cast<AFPSCharacter>(Pawn);
	if (Pawn)
		Cast<AShatterPlayerController>(Pawn->GetController())->OnPlayerDie.AddUObject(this, &ABattlePlayerState::OnDie);
}

void ABattlePlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);
}

void ABattlePlayerState::OnKill()
{
	KillCount++;
}

void ABattlePlayerState::OnDie(APawn* Sender, FVector DeathPoint)
{
	KnockbackMultiplier = 1;
	DeathCount++;
	AFPSCharacter* Character = Cast<AFPSCharacter>(Sender);
	
	if (Character->LastDamagingPlayer)
		Character->LastDamagingPlayer->OnKill();

	Character->LastDamagingPlayer = nullptr;
}


void ABattlePlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABattlePlayerState, KillCount);
	DOREPLIFETIME(ABattlePlayerState, DeathCount);
	DOREPLIFETIME(ABattlePlayerState, KnockbackMultiplier);
}



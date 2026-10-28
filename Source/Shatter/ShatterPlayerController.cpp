// Fill out your copyright notice in the Description page of Project Settings.


#include "ShatterPlayerController.h"
#include "BattlePlayerState.h"

void AShatterPlayerController::OnPossess(APawn* PawnP)
{
	Super::OnPossess(PawnP);
	GetPlayerState<ABattlePlayerState>()->OnPossess(PawnP);
}

void AShatterPlayerController::EndGame_Implementation(bool bWon)
{
	Cast<UEOSGameInstance>(GetGameInstance())->EndGame(bWon);
}



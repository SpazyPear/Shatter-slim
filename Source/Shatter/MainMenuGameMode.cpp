// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"
#include "EOSGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MenuPlayerState.h"
#include <Kismet/GameplayStatics.h>


AMainMenuGameMode::AMainMenuGameMode()
{
}

void AMainMenuGameMode::BeginPlay()
{
	GameState = GetGameState<AMultiplayerGameState>();
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AMainMenuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void AMainMenuGameMode::Tick(float DeltaTime)
{

}


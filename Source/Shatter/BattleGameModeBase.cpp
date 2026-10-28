// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include <GameFramework/PlayerStart.h>
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "FPSCharacter.h"
#include "EngineUtils.h"
#include "EOSGameInstance.h"
#include "ShatterPlayerController.h"
#include "TimerManager.h"


ABattleGameModeBase::ABattleGameModeBase()
{
	//PlayerStateClass = ABattlePlayerState::StaticClass();
	DeadPlayers = TArray<ABattlePlayerState*>();
	SpawnPoints = TMap<AController*, FVector*>();
	HostRespawnPos = new FVector();
	ClientRespawnPos = new FVector();

}

void ABattleGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	TActorIterator<APlayerStart> PlayerStart = TActorIterator<APlayerStart>(GetWorld());

	State = GetGameState<AMultiplayerGameState>();
	
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

#if WITH_EDITOR
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &ABattleGameModeBase::StartGame, 5, false, 5);
#endif
}

void ABattleGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// If a player joins during a game, make them a spectator
void ABattleGameModeBase::PostLogin(APlayerController* C)
{
	if (GetGameState<AMultiplayerGameState>()->GameStage != EGameStage::WAITING)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Game already started, you are in spectator mode.")));
		ASpectatorPawn* SpectatorPawn = GetWorld()->SpawnActor<ASpectatorPawn>(SpectatorClass, *HostRespawnPos, FRotator::ZeroRotator, FActorSpawnParameters());
		C->Possess(SpectatorPawn);
		return;
	}
	SpawnPoints.Add(C, bHostSpawnSet ? ClientRespawnPos : HostRespawnPos);
	bHostSpawnSet = true;
	Cast<AShatterPlayerController>(C)->OnPlayerDie.AddUObject(this, &ABattleGameModeBase::OnDeath);
	Super::PostLogin(C);
}

// Handle Respawns
void ABattleGameModeBase::RestartPlayer(AController* C)
{
	Super::RestartPlayer(C);

	if (!bSpawnsSet) {
		int i = 0;
		for (auto it = TActorIterator<APlayerStart>(GetWorld()); it; ++it)
		{

			if (i > 0) {

				*HostRespawnPos = it->GetActorLocation();
			}
			else {
				*ClientRespawnPos = it->GetActorLocation();
			}
			i++;
		}

		bSpawnsSet = true;
		C->GetPawn()->SetActorLocation(*SpawnPoints[C]);
		
	}
	
}

void ABattleGameModeBase::StartGame()
{

#if !WITH_EDITOR
	if (State->PlayerArray.Num() < 2)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Not enough players to start the game.")));
		return;
	}
#endif
	
	State->GameStage = EGameStage::STARTED;
	for (APlayerState* C : State->PlayerArray) 
	{
		if (C) 
		{
			C->GetPlayerController()->GetCharacter()->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
			Cast<AFPSCharacter>(C->GetPlayerController()->GetCharacter())->RemoveStartWidget();
		}
	}
}

// Check if player has died enough for them to have lost, and if so, end the game
void ABattleGameModeBase::OnDeath(APawn* PlayerDied, FVector DeathPoint)
{
	ABattlePlayerState* PlayerState = PlayerDied->GetPlayerState<ABattlePlayerState>();
	if (PlayerState->DeathCount > NumDeaths)
	{
		DeadPlayers.Add(PlayerState);
		PlayerState->IsEliminated = true;

		if (DeadPlayers.Num() < NumPlayers - 1) 
		{
			// If there are more than two players, have this dead player be a spectator from now
			APlayerController* C = PlayerState->GetPlayerController();
			C->GetPawn()->Destroy();
			ASpectatorPawn* SpectatorPawn = GetWorld()->SpawnActor<ASpectatorPawn>(SpectatorClass, *HostRespawnPos, FRotator::ZeroRotator, FActorSpawnParameters());
			C->Possess(SpectatorPawn);
		}
		else
		{
			OnGameOver();
		}
	}
	else 
	{
		PlayerState->KnockbackMultiplier = 1;
		PlayerState->PlayerRef->UpdateKnockbackIndicator(PlayerState->KnockbackMultiplier);
		Respawn(PlayerState->GetPawn());
	}
}

void ABattleGameModeBase::Respawn(APawn* DeadPlayer)
{
	APawn* OtherPlayer = DeadPlayer;
	for (APlayerState* p : State->PlayerArray)
	{
		if (p != DeadPlayer->GetPlayerState()) OtherPlayer = p->GetPawn();
	}
	FVector FurthestSpawn = FVector();
	for (auto it = TActorIterator<APlayerStart>(GetWorld()); it; ++it)
	{
		if (FurthestSpawn == FVector())
		{
			FurthestSpawn = it->GetActorLocation();
		}
		else if (FVector::Dist(it->GetActorLocation(), OtherPlayer->GetActorLocation()) > FVector::Dist(FurthestSpawn, OtherPlayer->GetActorLocation()))
		{
			FurthestSpawn = it->GetActorLocation();
		}
	}

	Cast<AFPSCharacter>(DeadPlayer)->ToggleClientAuthoratitiveMovement(false);
	UE_LOG(LogTemp, Warning, TEXT("Respawning player at %s"), *FurthestSpawn.ToString());
	DeadPlayer->SetActorLocation(FurthestSpawn);
}

void ABattleGameModeBase::OnGameOver()
{
	for (APlayerState* PlayerState : State->PlayerArray) 
	{
		if (APawn* Character = PlayerState->GetPawn()) 
		{
			Character->Destroy();
			Cast<AShatterPlayerController>(PlayerState->GetPlayerController())->EndGame(!DeadPlayers.Contains(PlayerState));
		}
	}
}

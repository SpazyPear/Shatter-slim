// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FPSCharacter.h"
#include "BattlePlayerState.h"
#include "MultiplayerGameState.h"
#include "BattleGameModeBase.generated.h"

/**
 * 
 */


UCLASS()
class SHATTER_API ABattleGameModeBase : public AGameMode
{
	GENERATED_BODY()
public:

	ABattleGameModeBase();


	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void PostLogin(APlayerController* C) override;

	virtual void RestartPlayer(AController* C) override;

	UPROPERTY(EditAnywhere)
		float DeathHeight;

	UPROPERTY(EditAnywhere)
		int NumDeaths;

	AMultiplayerGameState* State;

	TArray<ABattlePlayerState*> DeadPlayers;

	TMap<AController*, FVector*> SpawnPoints;

	bool bSpawnsSet;

	bool bHostSpawnSet;


private:
	
	UFUNCTION(BlueprintCallable)
	void StartGame();

	void OnDeath(APawn* PlayerDied, FVector DeathPoint);

	void Respawn(APawn* DeadPlayer);

	void OnGameOver();
	
	FVector* HostRespawnPos;

	FVector* ClientRespawnPos;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MultiplayerGameState.h"
#include "MainMenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHATTER_API AMainMenuGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	AMainMenuGameMode();

	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Tick(float DeltaTime) override;

	AMultiplayerGameState* GameState;
};

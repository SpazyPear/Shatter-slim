// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MultiplayerGameState.generated.h"

UENUM(BlueprintType)
enum EGameStage 
{
	WAITING,
	STARTED,
	ENDING
};

/**
 * 
 */
UCLASS()
class SHATTER_API AMultiplayerGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	
	AMultiplayerGameState();

	EGameStage GameStage;

};

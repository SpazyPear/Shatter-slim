// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EOSGameInstance.h"
#include "ShatterPlayerController.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerDie, APawn*, FVector);

/**
 * 
 */
UCLASS()
class SHATTER_API AShatterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void OnPossess(APawn* PawnP) override;

	UFUNCTION(Client, Reliable)
	void EndGame(bool bWon);

	FOnPlayerDie OnPlayerDie;
	
};

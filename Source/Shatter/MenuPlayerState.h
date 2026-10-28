// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MenuPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SHATTER_API AMenuPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	
	FString PlayerUserName;
	
	void SetPlayerUserName();

};

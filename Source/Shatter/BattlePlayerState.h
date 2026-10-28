// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "BattlePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SHATTER_API ABattlePlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	ABattlePlayerState();

	virtual void BeginPlay() override;

	class AFPSCharacter* PlayerRef;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int32 KillCount;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int32 DeathCount;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	float KnockbackMultiplier;

	void OnPossess(APawn* Pawn);
	
	virtual void ClientInitialize(AController* C) override;
	
	void OnKill();

	UFUNCTION()
	void OnDie(APawn* Sender, FVector DeathPoint);

	bool IsEliminated;

	bool bIsHost;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Camera/CameraComponent.h>
#include "Dime.h"
#include "Revolver.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyShot, FHitResult, Hit);

UCLASS()
class SHATTER_API ARevolver : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARevolver();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Shoot();

	UFUNCTION(Server, Reliable)
	void ShatterServer(ADime* dime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TEnumAsByte<EObjectTypeQuery>> ShootObjects;

	UCameraComponent* Camera;

	bool bPushMode;

	UFUNCTION(Client, Reliable)
	void SetShotMode();

	UFUNCTION(BlueprintImplementableEvent)
		void ChangeGunModeVisuals(bool bPushShot);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Damage;
	
};

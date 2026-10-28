// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fragment.h"
#include "Curves/CurveFloat.h"
#include "Components/BoxComponent.h"
#include "FMODBlueprintStatics.h"
#include "FMODEvent.h"
#include "Dime.generated.h"

UCLASS()
class SHATTER_API ADime : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADime();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	class AFPSCharacter* HoldingPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int FragCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SplitForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float PullStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float PushStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AFragment> FragmentBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ImpulseRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SlipDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TEnumAsByte<EObjectTypeQuery>> ImpulseObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRuntimeFloatCurve ImpulseFalloff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket Jumping")
		float SelfPushStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket Jumping")
		float SelfPullStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket Jumping")
		float SelfImpulseRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket Jumping")
		FRuntimeFloatCurve SelfImpulseFalloff;
	
	void Shatter(FVector HitVector, AFPSCharacter* PlayerThatShot);

	void SetPolarity(bool bisPush);

	void FireImpulse(AFPSCharacter* PlayerThatShot);

	UFUNCTION(Server, Reliable)	
		void OnTossed();

	void StaggerReplication();

	void EnableReplication();

	UBoxComponent* Hitbox;

	// FMOD SOUNDS
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* ShatterFX;
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* WooshFX1;
	UPROPERTY(EditAnywhere, Category = "FMOD")
		class UFMODEvent* WooshFX2;

private:

	bool bPush;

	FFMODEventInstance InstanceWrapper;
	FFMODEventInstance InstanceWoosh;

};

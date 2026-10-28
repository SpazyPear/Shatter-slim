// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GetRichCurveAndEvaluate.generated.h"

/**
 * 
 */

UCLASS()
class SHATTER_API UGetRichCurveAndEvaluate : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Math|RuntimeFloatCurve")
	static float GetRuntimeFloatCurveValue(FRuntimeFloatCurve RuntimeCurve, float ImpulseRadius, float Distance);
};

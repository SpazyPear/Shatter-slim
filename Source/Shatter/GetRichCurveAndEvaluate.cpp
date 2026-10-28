// Fill out your copyright notice in the Description page of Project Settings.


#include "GetRichCurveAndEvaluate.h"

float UGetRichCurveAndEvaluate::GetRuntimeFloatCurveValue(FRuntimeFloatCurve RuntimeCurve, float ImpulseRadius, float Distance)
{
	return RuntimeCurve.GetRichCurve()->Eval(FMath::GetMappedRangeValueClamped(FVector2D(0, ImpulseRadius), FVector2D(0, 1), Distance));
}
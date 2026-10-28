// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

void AMenuPlayerState::SetPlayerUserName() 
{
	PlayerUserName = UKismetSystemLibrary::GetPlatformUserName();
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Player user name is %s"), *PlayerUserName));

}
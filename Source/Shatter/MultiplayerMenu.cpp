// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerMenu.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"
#include "EOSGameInstance.h"
#include "OnlineSubsystem.h"

void UMultiplayerMenu::MenuSetup()
{
	GameInstance = Cast<UEOSGameInstance>(GetGameInstance());
	GameInstance->Login();
	//EnableOptions();
	if (!GameInstance->bIsLoggedIn)
	{
		GameInstance->OnEpicLoginSuccessful.BindUObject(this, &ThisClass::EnableOptions);
	}
	else
	{
		EnableOptions();
	}
}

bool UMultiplayerMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	if (GetGameInstance())
	{
		Cast<UEOSGameInstance>(GetGameInstance())->OnNoSessionsFound.BindUObject(this, &UMultiplayerMenu::JoinFailed);
	}

	return true;
}

void UMultiplayerMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMultiplayerMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	RoomName = RoomNameInput->GetText().ToString();
	if (GameInstance)
	{
		GameInstance->CreateSession(FName(RoomName));
	}
}

void UMultiplayerMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	RoomName = RoomNameInput->GetText().ToString();
	if (GameInstance)
	{
		GameInstance->FindSessions(RoomName);
	}
}

void UMultiplayerMenu::EnableOptions()
{
	HostButton->SetIsEnabled(true);
	JoinButton->SetIsEnabled(true);
}

void UMultiplayerMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}


void UMultiplayerMenu::JoinFailed()
{
	JoinButton->SetIsEnabled(true);
}


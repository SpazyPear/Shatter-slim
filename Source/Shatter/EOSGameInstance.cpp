// Fill out your copyright notice in the Description page of Project Settings.

#include "EOSGameInstance.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "EngineGlobals.h"
#include "OnlineSubsystemModule.h"
#include "Kismet/GameplayStatics.h"
#include "BattlePlayerState.h"
#include "MultiplayerGameState.h"
#include "GameFramework/GameState.h"
#include "BattleGameModeBase.h"


UEOSGameInstance::UEOSGameInstance()
{
	MaxPlayers = 3;
}

void UEOSGameInstance::Init()
{

	Super::Init();

	//if (false) 
	//{
	//	FOnlineSubsystemModule Subsystem = FOnlineSubsystemModule();
	//	OnlineSubsystem = Subsystem.GetOnlineSubsystem("NULL");
	//	return;
	//}

	OnlineSubsystem = IOnlineSubsystem::Get();

	//if (!GEditor->bIsSimulatingInEditor)
	//Login();


	crosshair_length = 10.0f;
	crosshair_spread = 5.0f;
	crosshair_thickness = 5.0f;
	crosshair_color_r = 0.0f;
	crosshair_color_g = 1.0f;
	crosshair_color_b = 0.0f;
	crosshair_show_dot = false;
	fmod_vca_master = 1.0f;
	fmod_vca_bgm = 1.0f;
	fmod_vca_sfx = 1.0f;
}


void UEOSGameInstance::EndSession()
{

}


void UEOSGameInstance::Login()
{
	if (!bIsLoggedIn)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Log In Via Browser")));

		if (OnlineSubsystem)
		{
			if (IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
			{
				FOnlineAccountCredentials Credentials;
				Credentials.Id = FString();
				Credentials.Token = FString();
				Credentials.Type = FString("accountportal");

				Identity->OnLoginCompleteDelegates->AddUObject(this, &UEOSGameInstance::OnLoginComplete);
				Identity->Login(0, Credentials);
			}
		}
	}
	else
		OnEpicLoginSuccessful.ExecuteIfBound();
}

void UEOSGameInstance::CreateSession(FName RoomName)
{
	if (OnlineSubsystem) 
	{
		if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface()) 
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Creating Session: %s"), *(RoomName.ToString())));
			CurrentRoomName = RoomName;
			FOnlineSessionSettings SessionSettings;
			SessionSettings.bIsDedicated = false;
			SessionSettings.bIsLANMatch = false;
			SessionSettings.NumPublicConnections = MaxPlayers;
			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bAllowJoinViaPresence = true;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bUsesPresence = true;
			SessionSettings.BuildUniqueId = 1;
			//SessionSettings.bUseLobbiesIfAvailable = true; This is for steam, using epic
			SessionSettings.Set(SEARCH_KEYWORDS, RoomName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

			SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnCreateSessionComplete);
			SessionPtr->CreateSession(0, FName(RoomName), SessionSettings);
		}
	}
}

void UEOSGameInstance::FindSessions(FString RoomName)
{
	if (bIsLoggedIn && OnlineSubsystem) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Searching For: %s"), *RoomName));

		if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface()) 
		{
			CurrentRoomName = FName(*RoomName);
			SearchSettings = MakeShareable(new FOnlineSessionSearch());
			SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, RoomName, EOnlineComparisonOp::Equals);
			SearchSettings->MaxSearchResults = 5000;
			SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnFindSessionsComplete);
			SessionPtr->FindSessions(0, SearchSettings.ToSharedRef());
		}
	}
}

IOnlineSessionPtr UEOSGameInstance::GetSessionPtr()
{
	if (OnlineSubsystem)
		return OnlineSubsystem->GetSessionInterface();
	else
		return nullptr;
}

void UEOSGameInstance::LeaveGame()
{
	GetSessionPtr()->EndSession(CurrentRoomName);
	GetSessionPtr()->DestroySession(CurrentRoomName);
}

void UEOSGameInstance::EndGame(bool bWon)
{
	bWonLastGame = bWon;
	LeaveGame();
	UGameplayStatics::OpenLevel(this, FName("EndMap"));
}

void UEOSGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Session Created: %d %d"), bWasSuccessful, OnlineSubsystem->GetSessionInterface().IsValid()));
	
	if (bWasSuccessful) {
		if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
			UE_LOG(LogTemp, Warning, TEXT("Loading Map: %s"), *(FString::Printf(TEXT("%s?listen"), *GameMapPath)))
			GetWorld()->ServerTravel(FString::Printf(TEXT("%s?listen"), *GameMapPath));
		}
	}
}

void UEOSGameInstance::OnLoginComplete(int32 UserNum, bool bWasSuccessful, const FUniqueNetId& UserID, const FString& Error)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Logged In: %d"), bWasSuccessful));

	if (OnlineSubsystem)
	{
		if (IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
		{
			Identity->ClearOnLoginCompleteDelegates(0, this);
			if (bWasSuccessful) 
			{ 
				bIsLoggedIn = true; 
				OnEpicLoginSuccessful.ExecuteIfBound();
			}
		}
	}
}

void UEOSGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful && SearchSettings->SearchResults.Num() > 0) 
	{

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Sessions Found: %d"), SearchSettings->SearchResults.Num()));

		if (GetSessionPtr())
		{
			GetSessionPtr()->ClearOnFindSessionsCompleteDelegates(this);
			GetSessionPtr()->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnJoinSessionComplete);
			GetSessionPtr()->JoinSession(0, FName(CurrentRoomName), SearchSettings->SearchResults[0]);
		}
	}
	else 
	{
		OnNoSessionsFound.ExecuteIfBound();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("No Sessions Found")));
	}
}

void UEOSGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Session Joining Complete: %s"), *(SessionName.ToString())));

	FString ConnectionInfo = FString();
	if (GetSessionPtr()->GetResolvedConnectString(SessionName, ConnectionInfo) && !ConnectionInfo.IsEmpty()) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Session Joined")));

		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0)) 
		{
			GetSessionPtr()->ClearOnJoinSessionCompleteDelegates(this);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Travelling %s"), *ConnectionInfo));
			PC->ClientTravel(ConnectionInfo, TRAVEL_Absolute);
		}
	}
}



// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSGameInstance.generated.h"

DECLARE_DELEGATE(FOnNoSessionFound);
DECLARE_DELEGATE(FOnEpicLoginSucessful);
/**
 * 
 */
UCLASS()
class SHATTER_API UEOSGameInstance : public UGameInstance
{
	GENERATED_BODY()
		
public:
	UEOSGameInstance();
	
	virtual	void Init() override;

	UFUNCTION(BlueprintCallable)
		void Login();

	UFUNCTION(BlueprintCallable)
		void CreateSession(FName RoomName);

	UFUNCTION(BlueprintCallable)
		void FindSessions(FString RoomName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString GameMapPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaxPlayers;

	IOnlineSubsystem* OnlineSubsystem;

	bool bIsLoggedIn;

	TSharedPtr<FOnlineSessionSearch> SearchSettings;

	IOnlineSessionPtr GetSessionPtr();
	
	FOnNoSessionFound OnNoSessionsFound;

	FOnEpicLoginSucessful OnEpicLoginSuccessful;

	bool isHost;

	UFUNCTION(BlueprintCallable)
		void EndSession();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int PlayersJoined;

	UFUNCTION(BlueprintCallable)
		void LeaveGame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bWonLastGame;
	
	void EndGame(bool bWon);


	//Crosshair Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float crosshair_length;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float crosshair_spread;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float crosshair_thickness;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float crosshair_color_r;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float crosshair_color_g;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float crosshair_color_b;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool crosshair_show_dot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fmod_vca_master;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fmod_vca_bgm;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fmod_vca_sfx;

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnLoginComplete(int32 UserNum, bool bWasSuccessful, const FUniqueNetId& UserID, const FString& Error);

	void OnFindSessionsComplete(bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	FName CurrentRoomName;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include <Components/EditableTextBox.h>
#include "EOSGameInstance.h"
#include "OnlineSubsystem.h"
#include "Components/TextBlock.h"
#include "MultiplayerMenu.generated.h"

/**
 *
 */
UCLASS()
class SHATTER_API UMultiplayerMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
		void MenuSetup();

protected:

	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

private:

	UPROPERTY(meta = (BindWidget))
		class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
		UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
		UEditableTextBox* RoomNameInput;

	UFUNCTION()
		void HostButtonClicked();

	UFUNCTION()
		void JoinButtonClicked();

	UFUNCTION()
		void EnableOptions();

	void MenuTearDown();

	UFUNCTION()
		void JoinFailed();

	UEOSGameInstance* GameInstance;

	IOnlineSubsystem* MultiplayerSessionsSubsystem;

	FString RoomName;
};


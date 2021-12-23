// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Networking.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "StreamStarterGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class STREAMSTARTER_API AStreamStarterGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Twitch API")
	void sendTwitchLogin(FString oauth, FString twitchName);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Twitch API")
	void OnLoggedIn();	

private:
	FSocket* ListenerSocket;
	FTimerHandle timerHandle;

	FString OAuth;
	FString Channel;
	FString Username;

	bool loggedInSuccessfully;

	bool inCountdownState;

	void ReceivedData();

	bool ConnectToTwitch();
	
	void LoginToTwitch();

	void JoinChannel();

	bool SendString(FString msg);

	void ParseMessage(FString msg);

	void ReceivedChatMessage(FString UserName, FString message);
	
};

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

	UFUNCTION(BlueprintCallable, Category="Countdown")
	void setCountdownSeconds(int Seconds);

	UFUNCTION(BlueprintCallable, Category="Countdown")
	void startCountdown();

	UFUNCTION(BlueprintCallable, Category="Twitch API")
	FString getRecentMessage();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Twitch API")
	void OnLoggedIn();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Countdown")
	void CountdownChange();

	UFUNCTION(BlueprintCallable, Category = "Countdown")
	float getTimeLeft();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Countdown")
	void CountdownFinished();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Countdown")
	void StopEmotes();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Twitch API")
	void OnChatMessage();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Twitch API")
	void OnCommandMessage();

private:
	FSocket* ListenerSocket;
	FTimerHandle timerHandle;

	FTimerHandle CountdownTimerHandle;

	FString OAuth;
	FString Channel;
	FString Username;

	FString recentMessage;

	float TimeLeft = 120;

	bool loggedInSuccessfully;

	bool inCountdownState;

	void ReceivedData();

	bool ConnectToTwitch();
	
	void LoginToTwitch();

	void JoinChannel();

	bool SendString(FString msg);

	void ParseMessage(FString msg);

	void ReceivedChatMessage(FString UserName, FString message);

	void Countdown();

	void StopCountdown();
};

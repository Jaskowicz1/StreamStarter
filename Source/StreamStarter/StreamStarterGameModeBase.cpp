// Copyright Epic Games, Inc. All Rights Reserved.

#include "StreamStarterGameModeBase.h"

#include <string>

void AStreamStarterGameModeBase::sendTwitchLogin(FString oauth, FString twitchName)
{
	this->OAuth = oauth;
	this->Channel = twitchName;
	this->Username = twitchName;
	ConnectToTwitch();
	LoginToTwitch();
}

void AStreamStarterGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void AStreamStarterGameModeBase::ReceivedData()
{

	if(ListenerSocket == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Null pointer."));
		return;
	}
	
	TArray<uint8> ReceivedData;
	uint32 Size;
	bool Received = false;
	while (ListenerSocket->HasPendingData(Size))
	{

		UE_LOG(LogTemp, Warning, TEXT("Does this happen?"));
		
		Received = true;
		ReceivedData.SetNumUninitialized(Size);
		int32 Read;
		ListenerSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
	}

	FString fs = "";
	
	if (Received)
	{

		UE_LOG(LogTemp, Warning, TEXT("Received Message."));
		
		const std::string cstr(reinterpret_cast<const char*>(ReceivedData.GetData()), ReceivedData.Num());
		fs = FString(cstr.c_str());
	}

	if(fs != "")
	{
		ParseMessage(fs);
	}
}

void AStreamStarterGameModeBase::ParseMessage(FString msg)
{
	TArray<FString> lines;
	msg.ParseIntoArrayLines(lines);
	for (FString fs : lines)
	{

		UE_LOG(LogTemp, Warning, TEXT("PARSEMESSAGE | %s"), *fs);

		if(!loggedInSuccessfully)
		{
			if(fs.Contains("Welcome, GLHF!"))
			{
				UE_LOG(LogTemp, Warning, TEXT("Found login message!"));
				loggedInSuccessfully = true;
				JoinChannel();
				OnLoggedIn();
			}
		}
		else
		{
			TArray<FString> parts;
			fs.ParseIntoArray(parts, TEXT(":"));
			TArray<FString> meta;
			parts[0].ParseIntoArrayWS(meta);
			if (parts.Num() >= 2)
			{
				if (meta[0] == TEXT("PING"))
				{
					SendString(TEXT("PONG :tmi.twitch.tv"));
				}
				else if (meta.Num() == 3 && meta[1] == TEXT("PRIVMSG"))
				{
					FString message=parts[1];
					if (parts.Num() > 2)
					{
						for (int i = 2; i < parts.Num(); i++)
						{
							message += TEXT(":") + parts[i];
						}
					}
					FString username;
					FString tmp;
					meta[0].Split(TEXT("!"), &username, &tmp);
					ReceivedChatMessage(username, message);
					continue;
				}
			}
		}
	}
}


void AStreamStarterGameModeBase::ReceivedChatMessage(FString UserName, FString message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *UserName, *message);
	recentMessage = message;

	UE_LOG(LogTemp, Warning, TEXT("Checking countdown state."));
	if(inCountdownState)
	{
		UE_LOG(LogTemp, Warning, TEXT("In Countdown State"));
		
		if(message.StartsWith("!"))
		{
			OnCommandMessage();
		}
		else
		{
			OnChatMessage();
		}
	}
}

FString AStreamStarterGameModeBase::getRecentMessage()
{
	return recentMessage;
}


bool AStreamStarterGameModeBase::ConnectToTwitch()
{

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	TSharedRef<FInternetAddr> addr = SocketSubsystem->CreateInternetAddr();
	ESocketErrors SocketErrors = SocketSubsystem->GetHostByName("irc.twitch.tv", *addr);

	if(SocketErrors != SE_NO_ERROR)
	{
		// Can't resolve hostname.
		return false;
	}

	const int32 port = 6667;
	addr->SetPort(port);

	FSocket* fSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("Twitch Socket"), false);

	if(fSocket == nullptr)
	{
		// Could not create the socket.
		return false;
	}

	int32 outSize;
	fSocket->SetReceiveBufferSize(2 * 1024 * 1024, outSize);
	fSocket->SetReuseAddr(true);
	
	bool connected = fSocket->Connect(*addr);
	
	if (!connected)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to connect."));
		
		fSocket->Close();
		SocketSubsystem->DestroySocket(fSocket);
		
		return false;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Connected successfully!"));

	GetWorldTimerManager().SetTimer(timerHandle, this, &AStreamStarterGameModeBase::ReceivedData, 0.05f, true);
	this->ListenerSocket = fSocket;
	return true;
}


void AStreamStarterGameModeBase::LoginToTwitch()
{

	UE_LOG(LogTemp, Warning, TEXT("Sending Login."));

	SendString("PASS " + this->OAuth);
	SendString("NICK " + this->Username);
}

void AStreamStarterGameModeBase::JoinChannel()
{
	SendString("JOIN #" + this->Channel);
}


bool AStreamStarterGameModeBase::SendString(FString msg)
{
	if(ListenerSocket != nullptr && ListenerSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
	{
		msg += "\n";
		TCHAR* serializedChar = msg.GetCharArray().GetData();
		int32 size = FCString::Strlen(serializedChar);
		int32 OutSent;
		return ListenerSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, OutSent);
	}
	else
	{
		return false;
	}
}

// ------------------------------------------------------------------------

void AStreamStarterGameModeBase::setCountdownSeconds(int Seconds)
{
	TimeLeft = Seconds;
}

float AStreamStarterGameModeBase::getTimeLeft()
{
	return TimeLeft;
}


void AStreamStarterGameModeBase::startCountdown()
{
	inCountdownState = true;
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &AStreamStarterGameModeBase::Countdown, 0.001f, true);
}

void AStreamStarterGameModeBase::Countdown()
{
	TimeLeft -= .001f;
	
	CountdownChange();

	if(TimeLeft == 10)
	{
		StopEmotes();
	}
	else if(TimeLeft <= 0)
	{
		StopCountdown();
	}
}

void AStreamStarterGameModeBase::StopCountdown()
{
	inCountdownState = false;
	
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

	CountdownFinished();
}




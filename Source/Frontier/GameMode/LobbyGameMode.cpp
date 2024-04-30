// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Log a player has joined
	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		FString playerName = PlayerState->GetPlayerName();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan,
				FString::Printf(TEXT("%s has joined"), *playerName)
			);
		}
	}

	// Log the number of players in the lobby
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Emerald,
			FString::Printf(TEXT("Players in game: %d"), NumberOfPlayers)
		);
	}

	// Travel to the Game if the lobby is full
	if (NumberOfPlayers == 2) //Update to an editor variable
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Maps/DesertMap?listen")); //Update to blueprint variable
		}
	}

}

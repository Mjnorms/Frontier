// Mjnorms -- 2024

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

#include "Menu.generated.h"

#define SESSIONS_TO_FIND 100000 // Find a lot until we get a new steam dev app ID 

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup(TSoftObjectPtr<UWorld> lobbyLevel, int32 numConnections = 4, FString typeOfMatch = FString(TEXT("FFA")));
	void MenuSetup(FString lobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")), int32 numConnections = 4, FString typeOfMatch = FString(TEXT("FFA")));

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	// Multiplayer Subsystem delegate callbacks
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;
	UFUNCTION()
	void HostButtonClicked();
	UFUNCTION()
	void JoinButtonClicked();

	void MenuSetup_Internal();
	void MenuTeardown();

	// subsystem designed to handle all online func
	class UMultiplayerSessionsSubsystem* MultiplayerSessionSubsystem;

	// TODO: Update these variables with a server browser
	int32 m_numPublicConnections{ 4 };
	FString m_matchType{TEXT("FFA")};
	FString m_pathToLobby{TEXT("/Game/ThirdPerson/Maps/Lobby")};

};

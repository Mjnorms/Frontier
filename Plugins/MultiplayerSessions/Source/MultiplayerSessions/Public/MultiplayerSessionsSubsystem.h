// Mjnorms -- 2024

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

//keep this down here
#include "MultiplayerSessionsSubsystem.generated.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// New Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam	(FMultiplayerOnCreateSessionComplete	, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams		(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful);	//Cannot be Dynamic as FOnlineSearchResults is not a UObject :(
DECLARE_MULTICAST_DELEGATE_OneParam			(FMultiplayerOnJoinSessionComplete , EOnJoinSessionCompleteResult::Type result);										//Cannot be Dynamic as EOnJoinSessionCompleteResult is not a UEnum :(
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam	(FMultiplayerOnDestorySessionComplete	, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam	(FMultiplayerOnStartSessionComplete		, bool, bWasSuccessful);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLass
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()


public:
	UMultiplayerSessionsSubsystem();

	// Session Functionality
	void CreateSession(int32 numPublicConnections, FString matchType);
	void FindSessions(int32 numSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	// this class's delegates
	FMultiplayerOnCreateSessionComplete		MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete		MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete		MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestorySessionComplete	MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete		MultiplayerOnStartSessionComplete;

protected:
	// Internal Callbacks for the delegates
	void OnCreateSessionComplete(FName sessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type bWasSuccessful);
	void OnDestroySessionComplete(FName sessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName sessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// Online session Delegates (Don't forget to bind callbacks)
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	// member variables
	bool m_bcreateSessionOnDestroy{false};
	int32 m_lastNumPublicConnections;
	FString m_lastMatchType;
};

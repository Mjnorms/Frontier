// Mjnorms -- 2024

#include "MultiplayerSessionsSubsystem.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(	FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(	FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(	FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(	FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(	FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue,
				FString::Printf(TEXT("Found Subsytem %s"), *Subsystem->GetSubsystemName().ToString())
			);
		}
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 numPublicConnections, FString matchType)
{
	if (!SessionInterface.IsValid()) return;

	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Yellow,
		FString(TEXT("Creating Session"))
	);

	//check if session already exists, and destroy it if need be
	auto existingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (existingSession != nullptr)
	{
		m_bcreateSessionOnDestroy = true;
		m_lastNumPublicConnections = numPublicConnections;
		m_lastMatchType = matchType;
		DestroySession();
		return;
	}

	//Add delegate to interface delegate list, and store it in the handle
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//set session settings
	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	SessionSettings->NumPublicConnections = numPublicConnections;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->BuildUniqueId = 1;               //TODO: Update with a real build ID

	SessionSettings->Set(FName("MatchType"), matchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	//create Session
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool created = SessionInterface->CreateSession(*(localPlayer->GetPreferredUniqueNetId()), NAME_GameSession, *SessionSettings);
	if (!created)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 numSearchResults)
{
	if (!SessionInterface.IsValid()) return;

	//Add delegate to interface delegate list, save handle
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = numSearchResults;
	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	//find sessions
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool found = SessionInterface->FindSessions(*(localPlayer->GetPreferredUniqueNetId()), SessionSearch.ToSharedRef());
	if (!found)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& sessionResult)
{
	if (!SessionInterface.IsValid()) 
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		UE_LOG(LogTemp, Warning, TEXT("UMultiplayerSessionsSubsystem -> JoinSession : SessionInterface is not valid"));
		return;
	}

	// log 
	FString result_matchType;
	sessionResult.Session.SessionSettings.Get(FName("MatchType"), result_matchType);
	GEngine->AddOnScreenDebugMessage( -1, 15.f, FColor::Cyan,
		FString::Printf(TEXT("Attempting to join match type %s"), *result_matchType)
	);
	
	// join
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool joined = SessionInterface->JoinSession(*(localPlayer->GetPreferredUniqueNetId()), NAME_GameSession, sessionResult);
	if (!joined)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan,
			FString(TEXT("JoinSession failed"))
		);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		UE_LOG(LogTemp, Warning, TEXT("UMultiplayerSessionsSubsystem -> DestroySession : SessionInterface is not valid"));
		return;
	}
	DestroySessionCompleteDelegateHandle =  SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	bool destroyed = SessionInterface->DestroySession(NAME_GameSession);
	if (!destroyed)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan,
			FString(TEXT("DestroySession failed"))
		);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnStartSessionComplete.Broadcast(false);
		UE_LOG(LogTemp, Warning, TEXT("UMultiplayerSessionsSubsystem -> StartSession : SessionInterface is not valid"));
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan,
		FString(TEXT("Attempting to start match"))
	);

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
	bool started = SessionInterface->StartSession(NAME_GameSession);
	if (!started)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MultiplayerOnStartSessionComplete.Broadcast(false);
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan,
			FString(TEXT("StartSession failed"))
		);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED


void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (SessionSearch->SearchResults.Num() == 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	MultiplayerOnFindSessionsComplete.Broadcast(SessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	MultiplayerOnJoinSessionComplete.Broadcast(result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);

	//create a new session if we were waiting for the destroy to finish
	if (bWasSuccessful && m_bcreateSessionOnDestroy)
	{
		m_bcreateSessionOnDestroy = false;
		CreateSession(m_lastNumPublicConnections, m_lastMatchType);
	}
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}
	MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}
// Mjnorms -- 2024


#include "FrontierPlayerController.h"
#include "Frontier/HUD/PlayerHUD.h"
#include "Frontier/HUD/CharacterOverlay.h"
#include "Frontier/HUD/Annoucement.h"
#include "Frontier/GameMode/BlasterGameMode.h"
#include "Frontier/PlayerState/BlasterPlayerState.h"
#include "Frontier/GameState/BlasterGameState.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Frontier/PlayerCharacter.h"

void AFrontierPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD());
	ServerCheckMatchState();
}

void AFrontierPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFrontierPlayerController, MatchState);
}

void AFrontierPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void AFrontierPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AFrontierPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)	TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress)	TimeLeft = WarmupTime + MatchTime  - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown)	TimeLeft = CooldownTime + WarmupTime + MatchTime  - GetServerTime() + LevelStartingTime;

	// FIX for potential future time sync problems
	//if (HasAuthority())
	//{
	//	if (BlasterGameMode == nullptr)
	//	{
	//		BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	//		LevelStartingTime = BlasterGameMode->LevelStartingTime;
	//	}
	//	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	//	if (BlasterGameMode)
	//	{
	//		SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
	//	}
	//}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountDownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown) SetHUDAnnouncementCountdown(TimeLeft);
		if (MatchState == MatchState::InProgress) SetHUDMatchCountdown(SecondsLeft);
	}
	CountDownInt = SecondsLeft;
}

void AFrontierPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (PlayerHUD && PlayerHUD->CharacterOverlay)
		{
			CharacterOverlay = PlayerHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDeaths(HUDDeaths);
				SetHUDWeaponAmmo(HUDWeaponAmmo);
				SetHUDCarriedAmmo(HUDCarriedAmmo);
			}
		}
	}
}

void AFrontierPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}
void AFrontierPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
float AFrontierPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}
void AFrontierPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}


void AFrontierPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->HealthBar != nullptr && PlayerHUD->CharacterOverlay->HealthText != nullptr;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		PlayerHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
	else
	{
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}

}

void AFrontierPlayerController::SetHUDScore(float score)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->ScoreAmount != nullptr;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(score));
		PlayerHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
		HUDScore = score;
	}
	else
	{
		HUDScore = score;
	}

}

void AFrontierPlayerController::SetHUDDeaths(int32 Deaths)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->DeathsAmount != nullptr;
	if (bHUDValid)
	{
		FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		PlayerHUD->CharacterOverlay->DeathsAmount->SetText(FText::FromString(DeathsText));
		HUDDeaths = Deaths;
	}
	else
	{
		HUDDeaths = Deaths;
	}

}

void AFrontierPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->WeaponAmmoAmount != nullptr;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
		HUDWeaponAmmo = Ammo;
	}
	else
	{
		HUDWeaponAmmo = Ammo;
	}

}

void AFrontierPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->CarriedAmmoAmount != nullptr;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
		HUDCarriedAmmo = Ammo;
	}
	else
	{
		HUDCarriedAmmo = Ammo;
	}
}

void AFrontierPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->MatchCountdownText != nullptr;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AFrontierPlayerController::SetHUDAnnouncementCountdown(float AnnoucementTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->AnnouncementOverlay != nullptr && PlayerHUD->AnnouncementOverlay->WarmupTime != nullptr;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(AnnoucementTime / 60.f);
		int32 Seconds = AnnoucementTime - (Minutes * 60);
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->AnnouncementOverlay->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AFrontierPlayerController::DisplayDeathNotif()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->ScoreAmount != nullptr;
	if (!bHUDValid) return;

	PlayerHUD->CharacterOverlay->DeathNotif->SetVisibility(ESlateVisibility::Visible);
}

void AFrontierPlayerController::HideDeathNotif()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->ScoreAmount != nullptr;
	if (!bHUDValid) return;

	PlayerHUD->CharacterOverlay->DeathNotif->SetVisibility(ESlateVisibility::Hidden);
}


void AFrontierPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AFrontierPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AFrontierPlayerController::HandleMatchHasStarted()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr;
	if (bHUDValid)
	{
		PlayerHUD->AddCharacterOverlay();
		if (PlayerHUD->AnnouncementOverlay)
		{
			PlayerHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AFrontierPlayerController::HandleCooldown()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		if (PlayerHUD->CharacterOverlay)
			PlayerHUD->CharacterOverlay->RemoveFromParent();


		bool bHUDValid = PlayerHUD->AnnouncementOverlay &&
			PlayerHUD->AnnouncementOverlay->AnnouncementText &&
			PlayerHUD->AnnouncementOverlay->InfoText;
		if (bHUDValid)
		{
			PlayerHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Visible);

			// Header Text
			FString AnnouncementText("New Match Starts In:");
			PlayerHUD->AnnouncementOverlay->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			// Info Text
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState <ABlasterPlayerState>();
			FString InfoTextString = "";
			if (BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				if		(TopPlayers.Num() == 0)											InfoTextString = FString("No Winner");
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)  InfoTextString = FString("You Are The WINNER");
				else if (TopPlayers.Num() == 1)											InfoTextString = FString::Printf(TEXT("Winner:/n%s"), *TopPlayers[0]->GetPlayerName());
				else if (TopPlayers.Num() >  1)
				{
					InfoTextString = FString("Players tied for the win:\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
			}
			else
			{
				InfoTextString = "No Winner/nInvalid State";
			}
			PlayerHUD->AnnouncementOverlay->InfoText->SetText(FText::FromString(InfoTextString));
		}
	}
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->bDisableGameplay = true;
	}
}

void AFrontierPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		LevelStartingTime = GameMode->LevelStartingTime;
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		MatchState = GameMode->GetMatchState();
		if (PlayerHUD && MatchState == MatchState::WaitingToStart && PlayerHUD->AnnouncementOverlay == nullptr)
		{
			PlayerHUD->AddAnnoucement();
		}
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}

void AFrontierPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown )
{
	LevelStartingTime = StartingTime;
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	MatchState = StateOfMatch;
	if (PlayerHUD && MatchState == MatchState::WaitingToStart && PlayerHUD->AnnouncementOverlay == nullptr)
	{
		PlayerHUD->AddAnnoucement();
	}
}

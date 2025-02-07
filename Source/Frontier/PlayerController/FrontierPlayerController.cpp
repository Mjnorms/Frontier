// Mjnorms -- 2024


#include "FrontierPlayerController.h"
#include "Frontier/HUD/PlayerHUD.h"
#include "Frontier/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AFrontierPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD());
}

void AFrontierPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
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
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountDownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(SecondsLeft);
	}
	CountDownInt = SecondsLeft;
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
	if (!bHUDValid) return;

	const float HealthPercent = Health / MaxHealth;
	PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
	PlayerHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
}

void AFrontierPlayerController::SetHUDScore(float score)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->ScoreAmount != nullptr;
	if (!bHUDValid) return;

	FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(score));
	PlayerHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
}

void AFrontierPlayerController::SetHUDDeaths(int32 Deaths)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->DeathsAmount != nullptr;
	if (!bHUDValid) return;

	FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
	PlayerHUD->CharacterOverlay->DeathsAmount->SetText(FText::FromString(DeathsText));
}

void AFrontierPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->WeaponAmmoAmount != nullptr;
	if (!bHUDValid) return;

	FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
	PlayerHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
}

void AFrontierPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->CarriedAmmoAmount != nullptr;
	if (!bHUDValid) return;

	FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
	PlayerHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
}

void AFrontierPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD != nullptr && PlayerHUD->CharacterOverlay != nullptr && PlayerHUD->CharacterOverlay->MatchCountdownText != nullptr;
	if (!bHUDValid) return;

	int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
	int32 Seconds = CountdownTime - (Minutes * 60);
	FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
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

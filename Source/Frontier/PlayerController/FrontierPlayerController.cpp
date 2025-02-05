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

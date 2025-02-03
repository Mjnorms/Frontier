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

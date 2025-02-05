// Mjnorms -- 2024

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class FRONTIER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
// variables
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreAmount = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DeathsAmount = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DeathNotif = nullptr;
};

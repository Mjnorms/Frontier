// Mjnorms -- 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FrontierPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FRONTIER_API AFrontierPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	class APlayerHUD* PlayerHUD = nullptr;


protected:
	virtual void BeginPlay() override;

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void DisplayDeathNotif();
	void HideDeathNotif();
};

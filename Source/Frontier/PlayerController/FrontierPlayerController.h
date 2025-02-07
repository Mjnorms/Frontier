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

	virtual void Tick(float DeltaTime) override;

	float MatchTime = 120.f;
	int CountDownInt = 0;
	void SetHUDTime();
	// sync time
	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);
	float ClientServerDelta = 0.f; // difference between client and server time
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

protected:
	virtual void BeginPlay() override;

	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void DisplayDeathNotif();
	void HideDeathNotif();
};

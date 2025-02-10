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

	float LevelStartingTime = 0.f;
	float WarmupTime = 0.f;
	float MatchTime = 0.f;
	float CooldownTime = 0.f;
	int CountDownInt = 0;
	void SetHUDTime();
	void PollInit();
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

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	bool bInitializeCharacterOverlay = false;

protected:
	virtual void BeginPlay() override;

	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

	void HandleMatchHasStarted();
	void HandleCooldown();


	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// bind replicated props


	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float AnnoucementTime);
	void DisplayDeathNotif();
	void HideDeathNotif();

	void OnMatchStateSet(FName State);

private:

	// Cached HUD values
	float HUDHealth = -1.f;
	float HUDMaxHealth = -1.f;
	float HUDScore = -1.f;
	int32 HUDDeaths = 0;
	int32 HUDWeaponAmmo = 0;
	int32 HUDCarriedAmmo = 0;
};

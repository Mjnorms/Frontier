// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Frontier/PlayerController/FrontierPlayerController.h"
#include "Frontier/HUD/PlayerHUD.h"
#include "Frontier/Weapon/WeaponTypes.h"
#include "Frontier/Types/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 1000000.f;

class APlayerCharacter;
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FRONTIER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
public:	
	UCombatComponent();
	friend class APlayerCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	void SetFiring(bool bIsFiring);

	void Fire();
	bool CanFire();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();
	UFUNCTION(BlueprintCallable)
	void FinishReload();

	void UpdateAmmoValues();
	int32 AmountToReload();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUDCrosshairs(float DeltaTime);
	void InterpFOV(float DeltaTime);
private:
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	UFUNCTION()
	void OnRep_CombatState();

	APlayerCharacter* PlayerCharacter = nullptr;
	class AFrontierPlayerController* PlayerController = nullptr;
	class APlayerHUD* HUD = nullptr;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	// reloading
	FTimerHandle ReloadSafetyTimer;
	void StartReloadSafetyTimer();
	void ReloadSafetyTimerFinished();

	// aiming
	UPROPERTY(Replicated)
	bool bAiming;
	float DefaultFOV;
	float CurrentFOV;
	UPROPERTY(EditAnywhere, Category = Combat)
	float UnZoomInterpSpeed = 20.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool AimingDebugDraw = false;

	// firing
	bool bFiring;
	FTimerHandle FireTimer;
	void StartFireTimer();
	void FireTimerFinished();
	bool bJustFired = false;

	// Carried Ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo = 0;
	UFUNCTION()
	void OnRep_CarriedAmmo();
	UPROPERTY(EditAnywhere)
	TMap<EWeaponType, int32> CarriedAmmoMap;

	// walk speed
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed = 600.f;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed = 450.f;

	
	// HUD and crosshairs
	FHUDPackage NewHUDPackage;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	FVector HitTarget;

public:	

		
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Frontier/Types/TurningInPlace.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Frontier/Interfaces/InteractWithCrosshairInterface.h"
#include "Components/TimelineComponent.h"
#include "Frontier/Types/CombatState.h"

#include "PlayerCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;


UCLASS()
class FRONTIER_API APlayerCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()
public:
	APlayerCharacter();
	void Destroyed() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;			// bind inputs

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// bind replicated props

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();
	void Elim();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCastElim();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

protected:
	virtual void BeginPlay() override;

	void InitControllerMappingContext();

	// Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* PlayerMappingContext = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* EquipAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* CrouchAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AimAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* FireAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ReloadAction = nullptr;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void EquipPressed();
	void CrouchPressed(const FInputActionValue& Value);
	void ReloadPressed(const FInputActionValue& Value);
	void AimPressed(const FInputActionValue& Value);
	void FirePressed(const FInputActionValue& Value);
	void SkipPressed(const FInputActionValue& Value);
	
	void AimOffset(float dt);

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// Poll for any relevant classes and init
	void InitHUD_Poll();
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon = nullptr;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat = nullptr;

	class AFrontierPlayerController* FrontierPlayerController = nullptr;
	class ABlasterPlayerState* BlasterPlayerState = nullptr;

	// RPC
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	// Aim Offset
	float AO_Pitch;
	float AO_Yaw;
	float Interp_AO_Yaw;
	FRotator StartingAimRotation;
	void HideCharacterIfCameraClose();
	UPROPERTY(EditAnywhere, Category = Camera)
	float MinCameraDistance = 150.f;

	// Turning the character when standing still but looking around
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float dt);

	// Anim Montages
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage = nullptr;
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage = nullptr;
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ElimMontage = nullptr;
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadMontage = nullptr;

	// Health
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	bool bElimd = false;
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	FTimerHandle ElimTimer;
	UFUNCTION()
	void OnRep_Health();
	void UpdateHUDHealth();
	void DisplayDeathNotification();
	void HideDeathNotification();
	void ElimTimerFinished();

	// Elim Dissolve
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline = nullptr;
	FOnTimelineFloat DissolveTrack;
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void InitializeDissolveMaterialParameters();
	void StartDissolve();
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	// Dynamic instance that we can change at runtime. 
	UPROPERTY(VisibleAnywhere, Category = "Eliminated")
	TArray<UMaterialInstanceDynamic*> DynamicDissolveMaterialInstances;
	// Material instance set on the blueprint, used with the dynamic material instance. 
	UPROPERTY(EditAnywhere, Category = "Eliminated")
	TArray<UMaterialInstance*> DissolveMaterialInstances;

public:  // Getters + Setters
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE bool IsElimd() const { return bElimd;  };
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FVector GetHitTarget() const;
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
};

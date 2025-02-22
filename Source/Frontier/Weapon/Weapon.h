// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"

#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX"),
};

UCLASS()
class FRONTIER_API AWeapon : public AActor
{
	GENERATED_BODY()
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);
	void WeaponUpdateHUD();
	bool IsEmpty();

protected:
	virtual void BeginPlay() override;

	UFUNCTION() 
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation = nullptr;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo, Category = "Ammo")
	int32 Ammo;

	void SpendRound();

	UFUNCTION()
	void OnRep_Ammo();

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MagCapacity;

	class APlayerCharacter* OwnerCharacter = nullptr;
	class AFrontierPlayerController* OwnerController = nullptr;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, Category = "Equip")
	class USoundBase* EquipSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Fire")
	class USoundBase* FireSound = nullptr;

public:
	// Crosshairs
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsCenter = nullptr;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsLeft = nullptr;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsRight = nullptr;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsTop = nullptr;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsBottom = nullptr;

	// Aiming FOV
	UPROPERTY(EditAnywhere, Category = "Aiming")
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere, Category = "Aiming")
	float ZoomInterpSpeed = 20.f;

	// Firing Timing
	UPROPERTY(EditAnywhere, Category = "FiringTiming")
	float FireDelay = 0.35f;
	UPROPERTY(EditAnywhere, Category = "FiringTiming")
	bool bAutomatic = true;

	// Reloading Timing (safety only)
	UPROPERTY(EditAnywhere, Category = "ReloadingTiming")
	float ReloadSafetyTime = 5.0f;

	// Sounds
	void PlayEquipSound();
	void PlayFireSound();

// Getters/Setters
public:
	void SetWeaponState(EWeaponState State);

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE bool HasEquipSound() const { return EquipSound == nullptr; }

};

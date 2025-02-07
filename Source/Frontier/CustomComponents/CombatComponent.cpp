// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Frontier/Weapon/Weapon.h"
#include "Frontier/PlayerCharacter.h"
#include "Frontier/PlayerController/FrontierPlayerController.h"
#include "Frontier/HUD/PlayerHUD.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"


UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.
	PrimaryComponentTick.bCanEverTick = true;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerCharacter && PlayerCharacter->GetFollowCamera())
	{
		if (PlayerCharacter->GetFollowCamera())
		{
			DefaultFOV = PlayerCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Setup to draw line from muzzle tip to hit target
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (PlayerCharacter == nullptr || WeaponToEquip == nullptr) return;
	
	//Drop curr wep if already holding one
	if (EquippedWeapon) EquippedWeapon->Dropped();

	//Setting new Equipped Weapon & state
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//Attach to Hand
	const USkeletalMeshSocket* HandSocket = PlayerCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, PlayerCharacter->GetMesh());
	}
	//SetOwner is already replicated, as owner has a rep notify OnRep_Owner()
	EquippedWeapon->SetOwner(PlayerCharacter);
	EquippedWeapon->WeaponUpdateHUD();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	PlayerController = PlayerController == nullptr ? Cast<AFrontierPlayerController>(PlayerCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}

	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	PlayerCharacter->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && PlayerCharacter)
	{
		//Setting Equipped Weapon & state
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		//Attach to Hand
		const USkeletalMeshSocket* HandSocket = PlayerCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, PlayerCharacter->GetMesh());
		}

		PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		PlayerCharacter->bUseControllerRotationYaw = true;

		EquippedWeapon->PlayEquipSound();
	}
}


void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	}
}

void UCombatComponent::Reload()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmo > 0 
		&& EquippedWeapon->GetAmmo() < EquippedWeapon->GetMagCapacity() 
		&& CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}
void UCombatComponent::ServerReload_Implementation()
{
	if (PlayerCharacter == nullptr) return;
	CombatState = ECombatState::ECS_Reloading;
	StartReloadSafetyTimer();
	HandleReload();
}

void UCombatComponent::HandleReload()
{
	PlayerCharacter->PlayReloadMontage();
}

void UCombatComponent::FinishReload()
{
	if (PlayerCharacter == nullptr) return;
	if (PlayerCharacter->HasAuthority())
	{
		PlayerCharacter->GetWorldTimerManager().ClearTimer(ReloadSafetyTimer);
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (PlayerCharacter == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<AFrontierPlayerController>(PlayerCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::StartReloadSafetyTimer()
{
	if (EquippedWeapon == nullptr || PlayerCharacter == nullptr) return;
	PlayerCharacter->GetWorldTimerManager().SetTimer(ReloadSafetyTimer, this, &UCombatComponent::ReloadSafetyTimerFinished, EquippedWeapon->ReloadSafetyTime);
}

void UCombatComponent::ReloadSafetyTimerFinished()
{
	if (CombatState == ECombatState::ECS_Reloading)
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPos;
	FVector CrosshairWorldDir;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPos,
		CrosshairWorldDir
	);

	if (bScreenToWorld)
	{
		bool AimingDebugDraw = false;
		FVector Start = CrosshairWorldPos;
		// Cast from the end of the gun if we can
		if (EquippedWeapon && EquippedWeapon->GetWeaponMesh()->DoesSocketExist(FName("MuzzleFlash")))
		{
			Start = EquippedWeapon->GetWeaponMesh()->GetSocketLocation(FName("MuzzleFlash"));
		}
		else if (PlayerCharacter)
		{
			// this is problematic bc player location is at the character's feet... different distance when looking up or down
			float DistanceToCharacter = (PlayerCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDir * (DistanceToCharacter + 60.f);
		}
		if (AimingDebugDraw) DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red, false);

		FVector End = Start + CrosshairWorldDir * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		if (AimingDebugDraw)
		{
			DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12.f,
				FColor::Red
			);
		}
		// Crosshair Interaction
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
		{
			NewHUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			NewHUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (PlayerCharacter == nullptr) return;
	PlayerController = PlayerController == nullptr ? Cast<AFrontierPlayerController>(PlayerCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		HUD = HUD == nullptr ? Cast<APlayerHUD>(PlayerController->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				NewHUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				NewHUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				NewHUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				NewHUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				NewHUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			// Calculate crosshair spread    [0, 600] -> [0, 1]
			FVector2D WalkSpeedRange;
			FVector2D VelocityMultiplierRange;
			if (PlayerCharacter->bIsCrouched) //crouching
			{
				WalkSpeedRange = FVector2D(0.f, PlayerCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched);
				VelocityMultiplierRange = FVector2D(0.f, 0.5f);
			}
			else // walking
			{
				WalkSpeedRange = FVector2D(0.f, PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed);
				VelocityMultiplierRange = FVector2D(0.f, 1.f);
			}
			FVector Velocity = PlayerCharacter->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// Jumping / Falling
			if (PlayerCharacter->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
			NewHUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			HUD->SetHUDPackage(NewHUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->ZoomedFOV, DeltaTime, EquippedWeapon->ZoomInterpSpeed);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, UnZoomInterpSpeed);
	}
	if (PlayerCharacter && PlayerCharacter->GetFollowCamera())
	{
		PlayerCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetFiring(bool bIsFiring)
{
	if (EquippedWeapon == nullptr) return;
	bFiring = bIsFiring;
	if (bFiring && CanFire())
		Fire();

	if (!bFiring && !EquippedWeapon->bAutomatic)
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(FireTimer))
		{
			bJustFired = false;
		}
	}
}

void UCombatComponent::Fire()
{
	ServerFire(HitTarget);
	if (EquippedWeapon) CrosshairShootingFactor = .75f;
	StartFireTimer();
	bJustFired = true;
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && !bJustFired && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || PlayerCharacter == nullptr) return;
	PlayerCharacter->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon != nullptr && EquippedWeapon->bAutomatic) bJustFired = false;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<AFrontierPlayerController>(PlayerCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (PlayerCharacter && CombatState == ECombatState::ECS_Unoccupied)
	{
		PlayerCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		EquippedWeapon->PlayFireSound();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}


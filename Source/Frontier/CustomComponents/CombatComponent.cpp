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

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		PlayerCharacter->bUseControllerRotationYaw = true;
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
		/*
		* POTENTIAL ALTERNATIVE TO MAGIC NUMBER FOR MOVING TRACE FORWARD
		if (EquippedWeapon && EquippedWeapon->GetWeaponMesh()->DoesSocketExist(FName("MuzzleFlash")))
		{
			FVector MuzzleLocation = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash")).GetLocation();
			float DistanceToMuzzle = (MuzzleLocation - Start).Size();
			Start += CrosshairWorldDir * DistanceToMuzzle;
		}
		*/
		if (PlayerCharacter)
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
	PlayerController = PlayerController == nullptr ? Cast<APlayerController>(PlayerCharacter->Controller) : PlayerController;
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
	if (bFiring && !bJustFired)
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

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || PlayerCharacter == nullptr) return;
	PlayerCharacter->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon != nullptr && EquippedWeapon->bAutomatic) bJustFired = false;
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (PlayerCharacter)
	{
		PlayerCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (PlayerCharacter == nullptr || WeaponToEquip == nullptr) return;

	//Setting Equipped Weapon & state
	EquippedWeapon = WeaponToEquip; //TODO: Drop old wep
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//Attach to Hand
	const USkeletalMeshSocket* HandSocket = PlayerCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, PlayerCharacter->GetMesh());
	}
	//SetOwner is already replicated, as owner has a rep notify OnRep_Owner()
	EquippedWeapon->SetOwner(PlayerCharacter);

	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	PlayerCharacter->bUseControllerRotationYaw = true;
}


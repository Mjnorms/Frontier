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

void UCombatComponent::SetFiring(bool bIsFiring)
{
	bFiring = bIsFiring;
	if (bFiring)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
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
		FVector Start = CrosshairWorldPos;
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
		else
		{
			DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12.f,
				FColor::Red
			);
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
			FHUDPackage NewHUDPackage;
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

			NewHUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor;
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


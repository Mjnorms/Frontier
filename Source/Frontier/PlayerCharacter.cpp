// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "CustomComponents/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include "PlayerCharacterAnimInstance.h"
#include "Frontier/Frontier.h"
#include "Frontier/PlayerController/FrontierPlayerController.h"
#include "Frontier/PlayerState/BlasterPlayerState.h"
#include "Frontier/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Frontier/Weapon/WeaponTypes.h"


//////////////////////////////////////////////////////////////
// PUBLIC
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 400.f; //TODO: create a define for this
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Disable character collision with the camera (care, blueprints override easily)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	TurningInPlace = ETurningInPlace::ETIP_None;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void APlayerCharacter::Destroyed()
{
	Super::Destroyed();

	// Destroy equipped wep if not in progress
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APlayerCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(APlayerCharacter, Health);
	DOREPLIFETIME(APlayerCharacter, bDisableGameplay);
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->PlayerCharacter = this;
	}
}

void APlayerCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::PlayHitReactMontage()
{
	if (bElimd || Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName;
		// TODO: Directional Logic
		SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}


// Called on server only
void APlayerCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	MultiCastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &APlayerCharacter::ElimTimerFinished, ElimDelay);
}

// Called on all machines
void APlayerCharacter::MultiCastElim_Implementation()
{
	// anim
	bElimd = true;
	PlayElimMontage();

	// disolve
	InitializeDissolveMaterialParameters();
	StartDissolve();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay = true;

	// Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Display Death Notif
	DisplayDeathNotification();
}

void APlayerCharacter::InitializeDissolveMaterialParameters()
{
	DynamicDissolveMaterialInstances.SetNum(DissolveMaterialInstances.Num());

	for (int32 i = 0; i < DissolveMaterialInstances.Num(); ++i)
	{
		// Create dynamic material instance
		UMaterialInstanceDynamic* DynamicMat = UMaterialInstanceDynamic::Create(DissolveMaterialInstances[i], this);
		if (!DynamicMat) continue;

		DynamicDissolveMaterialInstances[i] = DynamicMat;
		GetMesh()->SetMaterial(i, DynamicMat);

		// Retrieve all scalar parameters from the material
		TArray<FMaterialParameterInfo> ScalarParamsInfo;
		TArray<FGuid> ScalarParamsIDs;
		DynamicMat->GetAllScalarParameterInfo(ScalarParamsInfo, ScalarParamsIDs);

		// Convert to a set for fast lookups
		TSet<FName> AvailableParams;
		for (const auto& ParamInfo : ScalarParamsInfo)
		{
			AvailableParams.Add(ParamInfo.Name);
		}

		// Desired parameters and values
		TMap<FName, float> ScalarParamsToSet = {
			{ TEXT("Dissolve"), 0.55f },
			{ TEXT("Glow"), 150.f }
		};

		// Apply only existing parameters
		for (const auto& Param : ScalarParamsToSet)
		{
			if (AvailableParams.Contains(Param.Key))
			{
				DynamicMat->SetScalarParameterValue(Param.Key, Param.Value);
			}
		}
	}

	int32 MaterialCount = GetMesh()->GetNumMaterials();
	// Set remaining materials to be fully dissolved (invisible)
	for (int32 i = DynamicDissolveMaterialInstances.Num(); i < MaterialCount; ++i)
	{
		UMaterialInterface* BaseMaterial = GetMesh()->GetMaterial(i);
		if (!BaseMaterial) continue;

		UMaterialInstanceDynamic* DynamicMat = UMaterialInstanceDynamic::Create(BaseMaterial, GetMesh());
		if (!DynamicMat) continue;

		GetMesh()->SetMaterial(i, DynamicMat);

		// Fully dissolve these materials
		DynamicMat->SetScalarParameterValue(TEXT("Opacity"), 0.0f);  // Fully transparent
	}
}

void APlayerCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void APlayerCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	for (const auto DynamicMaterial : DynamicDissolveMaterialInstances)
	{
		DynamicMaterial->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void APlayerCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &APlayerCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	InitHUD_Poll();

	AimOffset(DeltaTime);
	HideCharacterIfCameraClose();
}

//////////////////////////////////////////////////////////////
// PROTECTED

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &APlayerCharacter::ReceiveDamage);
	}
}


	//////////////////////////////////////////////////////////////////////////
	// Input
void APlayerCharacter::InitControllerMappingContext()
{
	FrontierPlayerController = FrontierPlayerController == nullptr ? Cast<AFrontierPlayerController>(GetController()) : FrontierPlayerController;
	if (FrontierPlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(FrontierPlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PlayerMappingContext, 0);
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::EquipPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void APlayerCharacter::CrouchPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (Value[0]) // Pressed
	{
		Crouch();
	}
	else          // Released
	{
		UnCrouch();
	}
}

void APlayerCharacter::ReloadPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (Value[0]) // Pressed
	{
		Combat->Reload();
	}
}

void APlayerCharacter::AimPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (!Combat || !Combat->EquippedWeapon) return;

	if (Value[0]) // Pressed
	{
		Combat->SetAiming(true);
	}
	else          // Released
	{
		Combat->SetAiming(false);
	}
}

void APlayerCharacter::FirePressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (!Combat || !Combat->EquippedWeapon) return;

	if (Value[0]) // Pressed
	{
		Combat->SetFiring(true);
	}
	else          // Released
	{
		Combat->SetFiring(false);
	}
}

void APlayerCharacter::AimOffset(float dt)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	FVector Velocity = GetVelocity();
	Velocity.Z = 0;
	float Speed = Velocity.Length();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_None)
		{
			Interp_AO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(dt);
	}
	if (Speed > 0.f || bIsInAir) // running or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_None;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}


void APlayerCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < MinCameraDistance)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void APlayerCharacter::TurnInPlace(float dt)
{
	if (AO_Yaw > 60.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -60.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_None)
	{
		Interp_AO_Yaw = FMath::FInterpTo(Interp_AO_Yaw, 0.f, dt, 4.f);
		AO_Yaw = Interp_AO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_None;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		//Equip
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &APlayerCharacter::EquipPressed);

		//Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &APlayerCharacter::CrouchPressed);

		//Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &APlayerCharacter::AimPressed);

		//Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &APlayerCharacter::FirePressed);

		//Reloading
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ReloadPressed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}

}

void APlayerCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

// called on server only
void APlayerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage(); // play montage on server

	if (Health == 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			FrontierPlayerController = FrontierPlayerController == nullptr ? Cast<AFrontierPlayerController>(GetController()) : FrontierPlayerController;
			AFrontierPlayerController* AttackerController = Cast<AFrontierPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, FrontierPlayerController, AttackerController);
		}
	}
}

void APlayerCharacter::InitHUD_Poll()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState != nullptr) // reaches here only on first successful poll
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDeaths(0);
			UpdateHUDHealth();
			HideDeathNotification();

			InitControllerMappingContext();
		}
	}
}


void APlayerCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void APlayerCharacter::UpdateHUDHealth()
{
	FrontierPlayerController = FrontierPlayerController == nullptr ? Cast<AFrontierPlayerController>(GetController()) : FrontierPlayerController;
	if (FrontierPlayerController)
	{
		FrontierPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void APlayerCharacter::DisplayDeathNotification()
{
	FrontierPlayerController = FrontierPlayerController == nullptr ? Cast<AFrontierPlayerController>(GetController()) : FrontierPlayerController;
	if (FrontierPlayerController)
	{
		FrontierPlayerController->DisplayDeathNotif();
	}
}

void APlayerCharacter::HideDeathNotification()
{
	FrontierPlayerController = FrontierPlayerController == nullptr ? Cast<AFrontierPlayerController>(GetController()) : FrontierPlayerController;
	if (FrontierPlayerController)
	{
		FrontierPlayerController->HideDeathNotif();
	}
}

void APlayerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon) OverlappingWeapon->ShowPickupWidget(false);
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon) OverlappingWeapon->ShowPickupWidget(true);
	}
}

// Replication callbacks store and send the last value that was replicated
// so if LastWeapon != null, then we were previously looking at a weapon
// if we are getting the callback, then we know the value has changed (either to null or to a new weapon)
void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon) OverlappingWeapon->ShowPickupWidget(true);
	if (LastWeapon) LastWeapon->ShowPickupWidget(false);
}

bool APlayerCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool APlayerCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* APlayerCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector APlayerCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState APlayerCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

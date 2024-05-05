// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UPlayerCharacterAnimInstance::NativeUpdateAnimation(float dt)
{
	Super::NativeUpdateAnimation(dt);
	if (PlayerCharacter == nullptr) PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	if (PlayerCharacter == nullptr) return;

	Velocity = PlayerCharacter->GetVelocity();
	Velocity.Z = 0;
	Speed = Velocity.Length();

	UCharacterMovementComponent* PlayerCharacterMvmt = PlayerCharacter->GetCharacterMovement();

	bIsInAir = PlayerCharacterMvmt->IsFalling();

	bIsAccelerating = PlayerCharacterMvmt->GetCurrentAcceleration().Length() > 0.f;

	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();

	bIsCrouched = PlayerCharacter->bIsCrouched;

	bIsAiming = PlayerCharacter->IsAiming();

	// Offset Yaw + Lean for strafing
	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, dt, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PlayerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / dt;
	const float Interp = FMath::FInterpTo(Lean, Target, dt, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
}

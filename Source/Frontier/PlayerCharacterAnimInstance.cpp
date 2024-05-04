// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
}

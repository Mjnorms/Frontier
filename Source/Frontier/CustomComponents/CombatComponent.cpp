// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Frontier/Weapon/Weapon.h"
#include "Frontier/PlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"


UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.
	PrimaryComponentTick.bCanEverTick = false;

}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (PlayerCharacter == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip; //TODO: Drop old wep
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = PlayerCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, PlayerCharacter->GetMesh());
	}
	EquippedWeapon->SetOwner(PlayerCharacter);
	EquippedWeapon->ShowPickupWidget(false);
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Frontier/Weapon/Weapon.h"
#include "Frontier/PlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"



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

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
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
}

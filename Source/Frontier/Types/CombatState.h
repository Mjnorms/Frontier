#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unnocupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),


	ECS_MAX UMETA(DisplayName = "Default_MAX")
};
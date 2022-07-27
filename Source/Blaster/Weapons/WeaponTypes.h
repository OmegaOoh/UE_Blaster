#pragma once

#define TRACE_LENGTH 80000.f;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssultRifle UMETA(DisplayName = "Assult Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SMG UMETA(DisplayName = "Submachine Gun"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_Sniper UMETA(DisplayName = "Sniper Rifle"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),




	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Components/ActorComponent.h"
#include "Blaster/Weapons/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapon();
	void Reload();
	void UpdateAmmoValues();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinish();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);


	void PickupAmmo(EWeaponType AmmoType,int32 PickupAmount);

protected:
	virtual void BeginPlay() override;

	/*
	 * Equip
	 */
	void ReloadEmptyWeapon();
	void PlayEquipSound(AWeapon* Weapon);
	void UpdateWeaponHUD();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToSling(AActor* ActorToAttach);
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	UFUNCTION()
		void OnRep_EquipWeapon();
	UFUNCTION()
		void OnRep_Secondary();

	/*
	 * Aiming
	 */
	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	/*
	 * Fire
	 */
	
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	/*
	 * Crosshair
	 */
	void TraceUnderCrosshair(FHitResult& TraceHitResult);
	void SetHUDCrosshair(float DeltaTime);

	/*
	 * Reload
	 */
	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();
	int32 AmountToReload();

	/*
	 * Grenade
	 */
	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	void ShowAttachedGrenade(bool bShowGrenade);
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;


private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquipWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Secondary)
	AWeapon* SecondaryWeapon;

	UPROPERTY(Replicated)
		bool bAiming;

	UPROPERTY(EditAnywhere)
		float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
		float AimWalkSpeed;

	UPROPERTY(EditAnywhere)
		float BaseCrouchSpeed;

	UPROPERTY(EditAnywhere)
		float AimCrouchSpeed;
	



	bool bFireButtonPressed;

	/*
	 * HUD and Crosshair
	*/
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	FHUDPackage HUDPackage;



	/*
	 * Aiming and FOV
	 */

	//FOV while Not Aiming; Set to the camera's base FOV in BeginPlay
	float DefaultFOV;
	
	UPROPERTY(EditAnyWhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnyWhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;


	void InterpFOV(float DeltaTime);

	/*
	 * Automatic Fire
	 */

	FTimerHandle FireTimer;

	bool bCanFire = true;
	bool CanFire();

	void StartFireTimer();
	void FireTimerFinished();

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	int32 MaxCarriedAmmoPerWeapon = 400;

	UPROPERTY(EditAnywhere,Category = WeaponStartAmmo)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere, Category = WeaponStartAmmo)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere, Category = WeaponStartAmmo)
	int32 StartingPistolAmmo = 14;

	UPROPERTY(EditAnywhere, Category = WeaponStartAmmo)
	int32 StartingSMGAmmo = 40;

	UPROPERTY(EditAnywhere, Category = WeaponStartAmmo)
	int32 StartingShotgunAmmo = 20;

	UPROPERTY(EditAnywhere, Category = WeaponStartAmmo)
	int32 StartingSniperAmmo = 8;

	UPROPERTY(EditAnywhere, Category = WeaponStartAmmo)
	int32 StartingGLAmmo = 12;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	void UpdateHUDGrenade();

public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool ShouldSwapWeapon();
};
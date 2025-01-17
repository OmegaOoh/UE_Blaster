// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_SecondaryEquipped UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun"),

	EFT_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);
	
	/*
	 * Automatic Fire
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
		float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = Combat)
		bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	/*
	 * Enable / Disable CustomDepth
	 */

	void EnableCustomDepth(bool bEnable);

	bool bDestroyedWeapon = false;

	UPROPERTY(EditAnywhere,Category =  "Weapon")
	EFireType FireType;

	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

	FVector TraceEndWithScatter(const FVector& HitTarget);


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

	UFUNCTION()
		void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);
	void OnWeaponStateSet();

	/*
	 * Trace end with Scatter
	 */

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere)
		float Damage = 20.f;

	UPROPERTY(EditAnywhere)
		float HeadShotDamage = 40.f;

	UPROPERTY(EditAnywhere,Replicated)
	bool bUseServerSideRewind = false;

	UPROPERTY()
		class ABlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()
		class ABlasterPlayerController* BlasterOwnerPlayerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
		EWeaponState WeaponState;

	UFUNCTION()
		void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere,Category = "Weapon Properties")
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	void SpendRound();

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 MagCapacity;

	//The Number of unprocessed server requests for Ammo
	//Incremented in SpendRound, Decremented in ClientUpdateAmmo
	int32 Sequence = 0;

	UPROPERTY(EditAnywhere, Category = Ammo)
	EWeaponType WeaponType;

	/*
	* Zoom FOV While Aiming
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Extension")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Extension")
	float ZoomInterpSpeed = 20.f;

	
public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UAnimationAsset* WeaponFireAnimation;

	void SpawnCasing();

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	bool IsEmpty();
	bool IsFull();
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }


/*
* Texture for The Weapon Crosshairs
*/

	UPROPERTY(EditAnywhere, Category = Crosshairs);
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs);
	class UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs);
	class UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs);
	class UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs);
	class UTexture2D* CrosshairsBottom;


};
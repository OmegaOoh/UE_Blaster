// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapons/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Blaster/Weapons/Projectile.h"
#include "Blaster/Weapons/Shotgun.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 400.f;

	BaseCrouchSpeed = 300.f;
	AimCrouchSpeed = 200.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}

	
	if(Controller)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(EquippedWeapon == nullptr)
		{
			Controller->SetHUDWeaponName(EWeaponType::EWT_MAX);
			Controller->SetHUDCarriedAmmo(0);
		}
		else
		{
			Controller->SetHUDWeaponName(EquippedWeapon->GetWeaponType());
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshair(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break;

	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapon:
		if(Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
	}
	
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			//Calculate Crosshair Spread
			
			//[0,600] -> [0,1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			

			if(Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (EquippedWeapon)
			{
				float ZoomedInterpSpeed = EquippedWeapon->GetZoomedInterpSpeed();
				if (bAiming)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, ZoomedInterpSpeed);
				}
				else
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, ZoomedInterpSpeed);
				}
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 20.f);

			HUDPackage.CrosshairSpread = 1.f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

/*
 * Equip and Pickup
 */

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	bCanFire = true;
}

void UCombatComponent::SwapWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr) return;

	Character->PlaySwapMontage();
	Character->bFinishedSwapping = false;
	CombatState = ECombatState::ECS_SwappingWeapon;


	AWeapon* LastEquippedWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = LastEquippedWeapon;
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;

	if (EquippedWeapon)
	{
		if (EquippedWeapon->bDestroyedWeapon)
		{
			EquippedWeapon->Destroy();
		}
		else
		{
			EquippedWeapon->Dropped();
		}
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(WeaponToEquip);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	UpdateWeaponHUD();
	PlayEquipSound(WeaponToEquip);
	ReloadEmptyWeapon();

}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr ) return;

	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_SecondaryEquipped);
	AttachActorToSling(WeaponToEquip);
	PlayEquipSound(WeaponToEquip);
	SecondaryWeapon->SetOwner(Character);
}

void UCombatComponent::OnRep_EquipWeapon()
{
	if (EquippedWeapon && Character && Controller)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		UpdateWeaponHUD();
		PlayEquipSound(EquippedWeapon);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_Secondary()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_SecondaryEquipped);
		AttachActorToSling(SecondaryWeapon);
		PlayEquipSound(SecondaryWeapon);
	}
}

void UCombatComponent::PlayEquipSound(AWeapon* Weapon)
{
	if ( Character && Weapon && Weapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::UpdateWeaponHUD()
{
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
		Controller->SetHUDWeaponName(EquippedWeapon->GetWeaponType());
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || ActorToAttach == nullptr ) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToSling(AActor* ActorToAttach)
{
	if (Character == nullptr || ActorToAttach == nullptr) return;
	bool bUseHolster =
		SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_SMG;

	bool bUseBackSlot = 
		SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_RocketLauncher ||
		SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_Sniper;
	FName SocketName;
	if (bUseHolster) { SocketName = FName("HolsterSlot"); }
	else if (bUseBackSlot) { SocketName = FName("BackSlot"); }
	else if (!bUseBackSlot && !bUseHolster) { SocketName = FName("SlingSocket"); }

	const USkeletalMeshSocket* SecondarySlot = Character->GetMesh()->GetSocketByName(SocketName);
	if(SecondarySlot)
	{
		SecondarySlot->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

/*
 * Aiming
 */
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if(Character)
	{
		if(Character->bIsCrouched)
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = bIsAiming ? AimCrouchSpeed : BaseCrouchSpeed;
		}
		else
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		}
	}
	if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	if (Character->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character->bIsCrouched)
	{
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = bIsAiming ? AimCrouchSpeed : BaseWalkSpeed;
	}
	else
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_Aiming()
{
	if(Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

/*
 * Fire
 */

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()	
{
	if (CanFire())
	{
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;

			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				FireProjectile();
				break;
			case EFireType::EFT_HitScan:
				FireHitScan();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectile()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		ServerFire(HitTarget,EquippedWeapon->FireDelay);
		if (!Character->HasAuthority()) LocalFire(HitTarget);
	}
}

void UCombatComponent::FireHitScan()
{
	if(EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargetsArray;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargetsArray);
		if(!Character->HasAuthority()) ShotGunLocalFire(HitTargetsArray);
		ServerShotgunFire(HitTargetsArray,EquippedWeapon->FireDelay);
	}
	
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargetsArray,
	float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}

	return true;
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargetsArray, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargetsArray);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargetsArray)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	ShotGunLocalFire(TraceHitTargetsArray);
}


void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
	bCanFire = false;
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	if(EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}

	return true;
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& ServerTraceHitTarget, float FireDelay)
{
	MulticastFire(ServerTraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& MultiCastTraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(MultiCastTraceHitTarget);
}

/*
 * Local Fire
 */

void UCombatComponent::LocalFire(const FVector_NetQuantize& LocalHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		EquippedWeapon->Fire(LocalHitTarget);
		Character->PlayFireMontage(bAiming);
	}
}

void UCombatComponent::ShotGunLocalFire(const TArray<FVector_NetQuantize>& LocalHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;
	
	if (CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReload = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(LocalHitTargets);
	}
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if( Character )
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		//Fix Problem When Nothing Block TraceLine 
		if (!TraceHitResult.bBlockingHit) TraceHitResult.ImpactPoint = End;

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractwithCrosshairInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false; 
	if (bLocallyReload) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;

}

/*
 * Reload
 */
void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReload)
	{
		ServerReload();
		HandleReload();
		bLocallyReload = true;
	}
}

void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	int32 ReloadAmount = AmountToReload();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
		EquippedWeapon->SetHUDAmmo();
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
	EquippedWeapon->SetHUDAmmo();
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr|| EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	if(!Character->IsLocallyControlled()) HandleReload();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	bLocallyReload = false;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::FinishedSwap()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if (Character) Character->bFinishedSwapping = true;
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
}

void UCombatComponent::FinishedSwapAttatchedWeapon()
{
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	UpdateWeaponHUD();
	AttachActorToRightHand(EquippedWeapon);
	PlayEquipSound(EquippedWeapon);
	ReloadEmptyWeapon();

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_SecondaryEquipped);
	AttachActorToSling(SecondaryWeapon);
	SecondaryWeapon->SetOwner(Character);

	if (Character) Character->bFinishedSwapping = true;
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

/*
 * Ammo Calculation
 */
void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		if(EquippedWeapon != nullptr)
		{
			Controller->SetHUDCarriedAmmo(CarriedAmmo);
		}
		else
		{
			Controller->SetHUDCarriedAmmo(0);
		}
				
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Sniper, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGLAmmo);
}

void UCombatComponent::PickupAmmo(EWeaponType AmmoType, int32 PickupAmount)
{
	if (CarriedAmmoMap.Contains(AmmoType))
	{
		CarriedAmmoMap[AmmoType] = FMath::Clamp(CarriedAmmoMap[AmmoType] + PickupAmount, 0, MaxCarriedAmmoPerWeapon);
		UpdateAmmoValues();
	}

	if(EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == AmmoType)
	{
		Reload();
	}
}

/*
 * Grenade
 */

void UCombatComponent::ThrowGrenade()
{
	if (Grenades <= 0) return;

	if (CombatState != ECombatState::ECS_Unoccupied) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		ShowAttachedGrenade(true);
	}
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();

	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenade();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades <= 0) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenade();
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenade();
}

void UCombatComponent::UpdateHUDGrenade()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenade(Grenades);
	}
}

bool UCombatComponent::ShouldSwapWeapon()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr) ;
}

void UCombatComponent::ThrowGrenadeFinish()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);

	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}

}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams);
		}
	}
}

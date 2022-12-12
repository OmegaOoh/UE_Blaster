// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpawnPoint.h"
#include "Blaster/Weapons/Weapon.h"

// Sets default values
AWeaponSpawnPoint::AWeaponSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AWeaponSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	if(HasAuthority())
	{
		SpawnPickup();
	}
}


void AWeaponSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponSpawnPoint::SpawnPickup()
{
	const int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<AWeapon>(PickupClasses[Selection], GetActorTransform());

		if (HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &AWeaponSpawnPoint::StartSpawnPickupTimer);
		}
	}
}

void AWeaponSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

void AWeaponSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::RandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(
		SpawnPickupHandle,
		this,
		&AWeaponSpawnPoint::SpawnPickupTimerFinished,
		SpawnTime
	);
}


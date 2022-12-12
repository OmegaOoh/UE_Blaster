// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponSpawnPoint.generated.h"

UCLASS()
class BLASTER_API AWeaponSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponSpawnPoint();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AWeapon>> PickupClasses;

	UPROPERTY(VisibleAnywhere)
	AWeapon* SpawnedPickup;

	void SpawnPickup();
	void SpawnPickupTimerFinished();
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

private:

	FTimerHandle SpawnPickupHandle;

	UPROPERTY(EditAnywhere)
		float SpawnPickupTimeMin;
	UPROPERTY(EditAnywhere)
		float SpawnPickupTimeMax;

public:




};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargetsArray);
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargetsArray);
private:

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberofPellets = 10;
};

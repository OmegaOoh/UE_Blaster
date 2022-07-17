// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();
protected:

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastPlayInAirSound(FVector Location );


	UPROPERTY( EditAnywhere,Category = "DamageRadius")
	float MinDamage = 10.f;
	UPROPERTY(EditAnywhere, Category = "DamageRadius")
	float DamageInnerRad = 200.f;
	UPROPERTY(EditAnywhere, Category = "DamageRadius")
	float DamageOuterRad = 500.f;

	UPROPERTY(EditAnyWhere)
	class USoundCue* RocketInAir;

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* RocketMesh;

	bool bIsHit = false;

};

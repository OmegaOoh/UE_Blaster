// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	/*
	 * Used with Server-Side Rewind
	 */

	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewind = false;

	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 25000.f;

	//Only set This for Grenade and Rocket
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	//Doesn't Matter for Grenade and Rocket
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor*OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere, Category = "DamageRadius")
	float MinDamage = 10.f;
	UPROPERTY(EditAnywhere, Category = "DamageRadius")
	float DamageInnerRad = 200.f;
	UPROPERTY(EditAnywhere, Category = "DamageRadius")
	float DamageOuterRad = 500.f;


	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer;

	UPROPERTY()
		class UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnyWhere)
		class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;

	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
		float DestroyTime = 3.f;

private:
	
public:	
	

};

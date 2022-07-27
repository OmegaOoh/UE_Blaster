// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/StaticMeshSocket.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileGrenade::AProjectileGrenade()
{
	/*
	 * Mesh is on Projectile.h
	 */

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	StartDestroyTimer();
	SpawnTrailSystem();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();

	Super::Destroyed();
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if(BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation());
	}
}



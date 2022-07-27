// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	
}


void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	bIsHit = true;
	if (OtherActor == GetOwner())
	{
		return;
	}
	StartDestroyTimer();

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileRocket::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector Location = GetActorLocation();


		if (bIsHit)
		{
			UGameplayStatics::PlaySoundAtLocation(this, RocketInAir, Location);
		}
}

void AProjectileRocket::Destroyed()
{
	ExplodeDamage();

	Super::Destroyed();
}

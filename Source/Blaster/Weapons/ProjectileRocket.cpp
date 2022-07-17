// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Sound/SoundCue.h"
#include "Engine/StaticMeshSocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	bIsHit = true;
	APawn* FiringPawn = GetInstigator();
	if(FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if(FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // World Context Obj.
				Damage, // BaseDamage
				MinDamage, // MinimumDamage
				GetActorLocation(), // Origin
				DamageInnerRad, // Damage Inner Radius
				DamageOuterRad, // Damage Outer Radius
				1.f, // Damage Falloff
				UDamageType::StaticClass(), // DamageType
				TArray<AActor*>(), // Ignore Actors
				this, // Damage Causer
				FiringController //InstigatorController
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileRocket::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector Location = GetActorLocation();


		if (bIsHit)
		{
			UGameplayStatics::PlaySoundAtLocation(this, RocketInAir, Location);
			//MultiCastPlayInAirSound(Location);
		}
}
void AProjectileRocket::MultiCastPlayInAirSound_Implementation(FVector Location)
{
	UGameplayStatics::PlaySoundAtLocation(this, RocketInAir, Location);
}


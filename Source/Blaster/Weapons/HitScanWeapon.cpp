// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "WeaponTypes.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"


void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();


	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		if (FireHit.bBlockingHit)
		{

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter && InstigatorController )
			{
				const float DamageToDeal = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						BlasterCharacter,
						DamageToDeal,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
				else if (!HasAuthority() && bUseServerSideRewind)
				{
					BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
					BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerPlayerController;
					if (BlasterOwnerPlayerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
					{
						BlasterOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
							BlasterCharacter,
							Start,
							HitTarget,
							BlasterOwnerPlayerController->GetServerTime() - BlasterOwnerPlayerController->SingleTripTime
							);
					}
				}
			}
			
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}

			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint
				);
			}

		}


		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector & TraceStart, const FVector & HitTarget, FHitResult & OutHit)
	{
		UWorld* World = GetWorld();
		FVector TraceEnd = TraceStart + (HitTarget - TraceStart) * 1.25f;

		if (World)
		{
			TraceEnd = TraceStart + (HitTarget - TraceStart) * 1.25f;

			World->LineTraceSingleByChannel(
				OutHit,
				TraceStart,
				TraceEnd,
				ECollisionChannel::ECC_Visibility
			);

			FVector BeamEnd = TraceEnd;

			if (OutHit.bBlockingHit)
			{
				BeamEnd = OutHit.ImpactPoint;
			}
			else
			{
				OutHit.ImpactPoint = TraceEnd;
			}

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					TraceStart,
					FRotator::ZeroRotator,
					true
				);

				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		};
	}



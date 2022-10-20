// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargetsArray)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlasSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlasSocket)
	{
		const FTransform SocketTransform = MuzzleFlasSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		//Maps hit character to number of times hit
		TMap<ABlasterCharacter*, uint32> HitMap;
		TMap<ABlasterCharacter*, uint32> HeadShotHitMap;
		for (FVector_NetQuantize HitTarget : HitTargetsArray)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget,FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(BlasterCharacter)) HeadShotHitMap[BlasterCharacter] ++;
					else HeadShotHitMap.Emplace(BlasterCharacter, 1);
				}
				else
				{
					if (HitMap.Contains(BlasterCharacter)) HitMap[BlasterCharacter] ++;
					else HitMap.Emplace(BlasterCharacter, 1);
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
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-0.5f, 0.5f)
					);
				}

			}
		}
		TArray<ABlasterCharacter*> HitCharacters;
		TMap<ABlasterCharacter*, float> DamageMap;
		//Calculate BodyShot Damage  by Multiplying Times Hit * Damage - Stored In DamageMap
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && InstigatorController)
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		//Calculate HeadShot Damage  by Multiplying Times Hit * HeadShot Damage - Stored In DamageMap
		for (auto HSHitPair : HeadShotHitMap)
		{
			if (HSHitPair.Key && InstigatorController)
			{
				if (HeadShotHitMap.Contains(HSHitPair.Key)) DamageMap[HSHitPair.Key] += HSHitPair.Value * HeadShotDamage;
				else HeadShotHitMap.Emplace(HSHitPair.Key, HSHitPair.Value * HeadShotDamage);
				HitCharacters.AddUnique(HSHitPair.Key);
			}
		}
		//Loop Though DamageMap to get Total Damage for Each Character
		for (auto DamagePair: DamageMap)
		{
			if(DamagePair.Key  && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key,
						Damage * DamagePair.Value, //Cal. Dmg in two for loop above.
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}

		//ServerSideRewind
		 if (!HasAuthority() && bUseServerSideRewind)
		 {
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
			BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerPlayerController;
			if (BlasterOwnerPlayerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
			{
				BlasterOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargetsArray,
					BlasterOwnerPlayerController->GetServerTime() - BlasterOwnerPlayerController->SingleTripTime
				);
			}
		 }
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargetsArray)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;


	for (uint32 i = 0; i < NumberofPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		float TraceLength = TRACE_LENGTH

		ToEndLoc = FVector(TraceStart + ToEndLoc * TraceLength / ToEndLoc.Size());

		HitTargetsArray.Add(ToEndLoc);
	}
}

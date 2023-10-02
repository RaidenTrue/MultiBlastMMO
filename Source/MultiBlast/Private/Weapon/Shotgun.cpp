// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerController/BlastPlayerController.h"
#include "BlastComponents/LagCompensationComponent.h"
#include "Sound/SoundCue.h"
#include "BlastCharacter.h"

/* Replaced with "FireShotgun() */
//void AShotgun::Fire(const FVector& HitTarget)
//{
//	AWeapon::Fire(HitTarget);
//
//	APawn* OwnerPawn = Cast<APawn>(GetOwner());
//
//	if (OwnerPawn == nullptr) { return; }
//
//	AController* InstigatorController = OwnerPawn->GetController();
//
//	if (!HasAuthority() && InstigatorController)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Instigator Valid."));
//	}
//
//	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
//
//	if (MuzzleFlashSocket)
//	{
//		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
//		FVector Start = SocketTransform.GetLocation();
//		//uint32 Hits = 0;
//
//		TMap<ABlastCharacter*, uint32> HitMap;
//
//		for (uint32 i = 0; i < NumberOfPellets; i++)
//		{
//			FHitResult FireHit;
//			WeaponTraceHit(Start, HitTarget, FireHit);
//
//			ABlastCharacter* BlastCharacter = Cast<ABlastCharacter>(FireHit.GetActor());
//			if (BlastCharacter && HasAuthority() && InstigatorController)
//			{
//				if (HitMap.Contains(BlastCharacter))
//				{
//					HitMap[BlastCharacter]++;
//				}
//
//				else
//				{
//					HitMap.Emplace(BlastCharacter, 1);
//				}
//
//				//++Hits;
//			}
//
//			if (ImpactParticles)
//			{
//				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
//			}
//
//			if (HitSound)
//			{
//				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f));
//			}
//
//			/* Previous Implementation for the loop. */
//			//FVector End = TraceEndWithScatter(Start, HitTarget);
//		}
//
//		for (auto HitPair : HitMap)
//		{
//			if (HitPair.Key && HasAuthority() && InstigatorController)
//			{
//				UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
//			}
//		}
//	}
//}

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (OwnerPawn == nullptr) { return; }
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");	
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		/* Maps Hit Character to number of times Hit. */
		TMap<ABlastCharacter*, uint32> HitMap;
		TMap<ABlastCharacter*, uint32> HeadShotHitMap;

		for (const FVector_NetQuantize& HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlastCharacter* BlastCharacter = Cast<ABlastCharacter>(FireHit.GetActor());
			if (BlastCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(BlastCharacter)) { HeadShotHitMap[BlastCharacter]++; }

					else { HeadShotHitMap.Emplace(BlastCharacter), 1; }
				}
				else
				{
					if (HitMap.Contains(BlastCharacter)) { HitMap[BlastCharacter]++; }

					else { HitMap.Emplace(BlastCharacter, 1); }
				}

				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}

				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f));
				}
			}
		}

		TArray<ABlastCharacter*> HitCharacters;

		/* Maps Character Hit to Total Damage. */
		TMap<ABlastCharacter*, float> DamageMap;

		/* Calculate BodyShot Damage by Multiplying Times Hit Times Damage -> Store in DamageMap. */
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && InstigatorController)
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);

				HitCharacters.AddUnique(HitPair.Key);
			}
		}

		/* Calculate HeadShot Damage by Multiplying Times Hit Times HeadShotDamage -> Store in DamageMap. */
		for (auto HeadShotHitPair : HeadShotHitMap)
		{
			if (HeadShotHitPair.Key)
			{
				if (DamageMap.Contains(HeadShotHitPair.Key)) { DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage; }
				else { DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage); }

				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}

		/* Loops through DamageMap, to get TotalDamage for each Character. */
		for (auto DamagePair : DamageMap)
		{
			if (DamagePair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key, // Character that was Hit.
						DamagePair.Value, // Damage Calculated in the 2 for loops above.
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}

		

		/* Server-Side Rewind. */
		if (!HasAuthority() && bUseServerSideRewind)
		{
			BlastOwnerCharacter = BlastOwnerCharacter == nullptr ? Cast<ABlastCharacter>(OwnerPawn) : BlastOwnerCharacter;
			BlastOwnerController = BlastOwnerController == nullptr ? Cast<ABlastPlayerController>(InstigatorController) : BlastOwnerController;

			if (BlastOwnerController && BlastOwnerCharacter && BlastOwnerCharacter->GetLagCompensation() && BlastOwnerCharacter->IsLocallyControlled())
			{
				BlastOwnerCharacter->GetLagCompensation()->ShotgunServerSideRewind(HitCharacters, Start, HitTargets, BlastOwnerController->GetServerTime() - BlastOwnerController->SingleTripTime);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) { return; }

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		/* Randomness, which is what can be executed inside the Loop. */
		const FVector RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVect;
		FVector ToEndLoc = EndLoc - TraceStart;

		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		/* No Need to call Weapon->TraceEndWithScatter. */
		/*HitTargets.Add(TraceEndWithScatter(HitTarget));*/

		HitTargets.Add(ToEndLoc);
	}
}

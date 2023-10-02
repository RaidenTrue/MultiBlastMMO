// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/SpeedPickup.h"
#include "BlastComponents/BuffComponent.h"
#include "BlastCharacter.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlastCharacter* BlastCharacter = Cast<ABlastCharacter>(OtherActor);

	if (BlastCharacter)
	{
		UBuffComponent* Buff = BlastCharacter->GetBuff();

		if (Buff)
		{
			Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}

	Destroy();
}

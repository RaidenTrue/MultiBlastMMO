// Fill out your copyright notice in the Description page of Project Settings.


#include "BlastAnimInstance.h"
#include "BlastCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlastAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlastCharacter = Cast<ABlastCharacter>(TryGetPawnOwner());
}

void UBlastAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlastCharacter == nullptr)
	{
		BlastCharacter = Cast<ABlastCharacter>(TryGetPawnOwner());
	}

	if (BlastCharacter == nullptr) { return; }

	FVector Velocity = BlastCharacter->GetVelocity();
	Velocity.Z = 0.f;

	Speed = Velocity.Size();

	bIsInAir = BlastCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = BlastCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bWeaponEquipped = BlastCharacter->IsWeaponEquipped();

	EquippedWeapon = BlastCharacter->GetEquippedWeapon();

	bIsCrouched = BlastCharacter->bIsCrouched;

	bAiming = BlastCharacter->IsAiming();

	TurningInPlace = BlastCharacter->GetTurningInPlace();

	bRotateRootBone = BlastCharacter->ShouldRotateRootBone();

	bElimmed = BlastCharacter->IsElimmed();

	bHoldingTheFlag = BlastCharacter->IsHoldingTheFlag();

	/* Offset Yaw for Strafing. */
	FRotator AimRotation = BlastCharacter->GetBaseAimRotation();

	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlastCharacter->GetVelocity());

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);

	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);

	YawOffset = DeltaRotation.Yaw;

	/* Lean Yaw for Movement Leaning. */
	CharacterRotationLastFrame = CharacterRotation;

	CharacterRotation = BlastCharacter->GetActorRotation();

	const FRotator DeltaDifference = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);

	const float Target = DeltaDifference.Yaw / DeltaTime;
	
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);

	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	/*if (!BlastCharacter->HasAuthority() && !BlastCharacter->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw %f: "), AimRotation.Yaw);
		UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f: "), MovementRotation.Yaw);
	}*/

	//UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f: "), MovementRotation.Yaw);

	AO_Yaw = BlastCharacter->GetAO_Yaw();

	AO_Pitch = BlastCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlastCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		FVector OutPosition;
		
		FRotator OutRotation;

		BlastCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);

		LeftHandTransform.SetLocation(OutPosition);

		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (BlastCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation()) - BlastCharacter->GetHitTarget());
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}

		/* DrawDebugLines - Enable when Needed. */
		/*FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));

		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlastCharacter->GetHitTarget(), FColor::Green);*/

	}

	bUseFABRIK = BlastCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;

	bool bFABRIKOverride = BlastCharacter->IsLocallyControlled() && BlastCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade && BlastCharacter->bFinishedSwapping;

	if (bFABRIKOverride)
	{
		bUseFABRIK = !BlastCharacter->IsLocallyReloading();
	}

	bUseAimOffsets = BlastCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlastCharacter->GetDisabledGameplay();

	bTransformRightHand = BlastCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlastCharacter->GetDisabledGameplay();
}

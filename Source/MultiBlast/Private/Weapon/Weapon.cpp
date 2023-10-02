// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimationAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlastComponents/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerController/BlastPlayerController.h"
#include "BlastCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "BlastCharacter.h"
#include "Weapon/Casing.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	/*AreaSphere->SetSphereRadius(200.f);*/
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);

	/* Placed the lines above out of this IfCheck for Lag Compensation. */
	/*if (HasAuthority())
	{
		
	}*/

	/*if (!HasAuthority())
	{
		FireDelay = .001f;
	}*/
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);

	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);

	/*DOREPLIFETIME(AWeapon, Ammo);*/
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));

		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* World = GetWorld();

			if (World)
			{
				World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}
	/* Moved out of HasAuthority() due to using Client-Side Prediction. */
	SpendRound();
	/*if (HasAuthority())
	{
		
	}*/
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);

	SetOwner(nullptr);
	BlastOwnerCharacter = nullptr;
	BlastOwnerController = nullptr;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) { return FVector(); }

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	const FVector RandVect = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVect;
	const FVector ToEndLoc = EndLoc - TraceStart;

	/*DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Green, true);
	DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), FColor::Blue, true);*/

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	ABlastCharacter* BlastCharacter = Cast<ABlastCharacter>(OtherActor);

	if (BlastCharacter && PickupWidget)
	{
		if (WeaponType == EWeaponType::EWT_Flag && BlastCharacter->GetTeam() == Team) { return; }

		if (BlastCharacter->IsHoldingTheFlag()) { return; }
		
		/*PickupWidget->SetVisibility(true);*/
		BlastCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlastCharacter* BlastCharacter = Cast<ABlastCharacter>(OtherActor);

	if (BlastCharacter)
	{
		if (WeaponType == EWeaponType::EWT_Flag && BlastCharacter->GetTeam() == Team) { return; }

		if (BlastCharacter->IsHoldingTheFlag()) { return; }

		/*PickupWidget->SetVisibility(false);*/
		BlastCharacter->SetOverlappingWeapon(nullptr);
	}
}

//void AWeapon::OnRep_Ammo()
//{
//	BlastOwnerCharacter = BlastOwnerCharacter == nullptr ? Cast<ABlastCharacter>(GetOwner()) : BlastOwnerCharacter;
//
//	if (BlastOwnerCharacter && BlastOwnerCharacter->GetCombat() && IsFull())
//	{
//		BlastOwnerCharacter->GetCombat()->JumpToShotgunEnd();
//	}
//
//	SetHUDAmmo();
//}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		BlastOwnerCharacter = nullptr;

		BlastOwnerController = nullptr;
	}
	else
	{
		BlastOwnerCharacter = BlastOwnerCharacter == nullptr ? Cast<ABlastCharacter>(Owner) : BlastOwnerCharacter;

		if (BlastOwnerCharacter && BlastOwnerCharacter->GetEquippedWeapon() && BlastOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	BlastOwnerCharacter = BlastOwnerCharacter == nullptr ? Cast<ABlastCharacter>(GetOwner()) : BlastOwnerCharacter;

	if (BlastOwnerCharacter)
	{
		BlastOwnerController = BlastOwnerController == nullptr ? Cast<ABlastPlayerController>(BlastOwnerCharacter->Controller) : BlastOwnerController;

		if (BlastOwnerController && HasAuthority())
		{
			BlastOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);

	SetHUDAmmo();
	
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}	
	else
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) { return; }
	Ammo = ServerAmmo;	
	--Sequence;

	Ammo -= Sequence;

	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);

	SetHUDAmmo();

	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) { return; }

	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);

	BlastOwnerCharacter = BlastOwnerCharacter == nullptr ? Cast<ABlastCharacter>(GetOwner()) : BlastOwnerCharacter;
	if (BlastOwnerCharacter && BlastOwnerCharacter->GetCombat() && IsFull())
	{
		BlastOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}

	SetHUDAmmo();
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	OnWeaponStateSet();
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		/*ShowPickupWidget(false);*/
		OnEquipped();
		break;

	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;

	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;

	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);

	BlastOwnerCharacter = BlastOwnerCharacter == nullptr ? Cast<ABlastCharacter>(GetOwner()) : BlastOwnerCharacter;

	if (BlastOwnerCharacter && bUseServerSideRewind)
	{
		BlastOwnerController = BlastOwnerController == nullptr ? Cast<ABlastPlayerController>(BlastOwnerCharacter->Controller) : BlastOwnerController;

		if (BlastOwnerController && HasAuthority() && !BlastOwnerController->HighPingDelegate.IsBound())
		{
			BlastOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	/*EnableCustomDepth(true);*/

	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}

	BlastOwnerCharacter = BlastOwnerCharacter == nullptr ? Cast<ABlastCharacter>(GetOwner()) : BlastOwnerCharacter;

	if (BlastOwnerCharacter && bUseServerSideRewind)
	{
		BlastOwnerController = BlastOwnerController == nullptr ? Cast<ABlastPlayerController>(BlastOwnerCharacter->Controller) : BlastOwnerController;

		if (BlastOwnerController && HasAuthority() && BlastOwnerController->HighPingDelegate.IsBound())
		{
			BlastOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	BlastOwnerCharacter = BlastOwnerCharacter == nullptr ? Cast<ABlastCharacter>(GetOwner()) : BlastOwnerCharacter;

	if (BlastOwnerCharacter && bUseServerSideRewind)
	{
		BlastOwnerController = BlastOwnerController == nullptr ? Cast<ABlastPlayerController>(BlastOwnerCharacter->Controller) : BlastOwnerController;

		if (BlastOwnerController && HasAuthority() && BlastOwnerController->HighPingDelegate.IsBound())
		{
			BlastOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}




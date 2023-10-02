// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileRocket.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Weapon/RocketMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);

	RocketMovementComponent->InitialSpeed = InitialSpeed;
	RocketMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileRocket::Destroyed()
{

}

#if WITH_EDITOR
void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileRocket, InitialSpeed))
	{
		if (RocketMovementComponent)
		{
			RocketMovementComponent->InitialSpeed = InitialSpeed;
			RocketMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	SpawnTrailSystem();

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(ProjectileLoop, GetRootComponent(), FName(), GetActorLocation(), EAttachLocation::KeepWorldPosition, false, 1.f, 1.f, 0.f, LoopingSoundAttenuation, (USoundConcurrency*)nullptr, false);
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner()) 
	{ 
		/*UE_LOG(LogTemp, Warning, TEXT("HitSelf...!!!!"));*/
		return;
	}

	ExplodeDamage();

	//APawn* FiringPawn = GetInstigator();

	//if (FiringPawn && HasAuthority())
	//{
	//	AController* FiringController = FiringPawn->GetController();

	//	if (FiringController)
	//	{
	//		UGameplayStatics::ApplyRadialDamageWithFalloff(
	//			this, // World Context Object.
	//			Damage, // BaseDamage.
	//			10.f, // MinimumDamage.
	//			GetActorLocation(), // Origin.
	//			200.f, // DamageInnerRadius.
	//			500.f, // DamageOuterRadius.
	//			1.f, // DamageFalloff
	//			UDamageType::StaticClass(), // DamageTypeClass. 
	//			TArray<AActor*>(), // IgnoredActors.
	//			this, // DamageCauser.
	//			FiringController); // InstigatorController.
	//	}
	//}

	StartDestroyTimer();
	/*Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);*/

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}

	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}


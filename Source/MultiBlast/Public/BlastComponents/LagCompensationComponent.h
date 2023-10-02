// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AWeapon;
class ABlastCharacter;
class ABlastPlayerController;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABlastCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<ABlastCharacter*, uint32> BodyShots;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABlastCharacter* Character;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIBLAST_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULagCompensationComponent();

	friend class ABlastCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	/* For HitScan Weapons. */

	FServerSideRewindResult ServerSideRewind(ABlastCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	/* For Projectile Weapons. */

	FServerSideRewindResult ProjectileServerSideRewind(ABlastCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	/* For Shotgun Weapon. */

	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABlastCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ABlastCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(ABlastCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<ABlastCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	void CacheBoxPositions(ABlastCharacter* HitCharacter, FFramePackage& OutFramePackage);

	void MoveBoxes(ABlastCharacter* HitCharacter, const FFramePackage& Package);

	void ResetHitBoxes(ABlastCharacter* HitCharacter, const FFramePackage& Package);

	void EnableCharacterMeshCollision(ABlastCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	void SaveFramePackage();

	FFramePackage GetFrameToCheck(ABlastCharacter* HitCharacter, float HitTime);

	/* For HitScan Weapons. */

	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABlastCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

	/* For Projectile Weapons. */

	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, ABlastCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	/* For Shotgun Weapon. */

	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

private:

	UPROPERTY()
	ABlastCharacter* Character;

	UPROPERTY()
	ABlastPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:	
	


};

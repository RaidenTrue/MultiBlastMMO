// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiBlast/BlastTypes/Team.h"
#include "GameFramework/Character.h"
#include "MultiBlast/BlastTypes/TurningInPlace.h"
#include "Interfaces/CrosshairInteractInterface.h"
#include "MultiBlast/BlastTypes/CombatState.h"
#include "Components/TimelineComponent.h"
#include "BlastCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

class AWeapon;
class UAnimMontage;
class AController;
class USoundCue;
class UNiagaraSystem;
class UBuffComponent;
class UBoxComponent;
class UNiagaraComponent;
class UCameraComponent;
class UWidgetComponent;
class UCombatComponent;
class USpringArmComponent;
class ABlastPlayerState;
class ABlastGameMode;
class ABlastPlayerController;
class ULagCompensationComponent;

UCLASS()
class MULTIBLAST_API ABlastCharacter : public ACharacter, public ICrosshairInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlastCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	virtual void OnRep_ReplicatedMovement() override;

	virtual void Destroyed();

	/* Play Animation Montages. */
	void PlayFireMontage(bool bAiming);

	void PlayReloadMontage();

	void PlayElimMontage();

	void PlayThrowGrenadeMontage();

	void PlaySwapMontage();

	void Elim(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);

	UPROPERTY(Replicated, VisibleAnywhere)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();

	void UpdateHUDShield();

	void UpdateHUDAmmo();

	void SpawnDefaultWeapon();

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;

	bool bFinishedSwapping = false;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team);

	/* Removed in Favour of using Variable Replication instead of an RPC. */

	/*UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();*/

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void Turn(float Value);

	void LookUp(float Value);

	void CrouchButtonPressed();

	void ReloadButtonPressed();

	void EquipButtonPressed();

	void AimButtonPressed();

	void AimButtonReleased();

	void GrenadeButtonPressed();

	void AimOffset(float DeltaTime);

	void CalculateAO_Pitch();

	void SimProxiesTurn();

	virtual void Jump() override;

	void FireButtonPressed();

	void FireButtonReleased();

	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	/* Poll for any Relevant Classes & Initialize the PlayerHUD. */
	void PollInit();

	void RotateInPlace(float DeltaTime);

	void DropOrDestroyWeapon(AWeapon* Weapon);

	void DropOrDestroyWeapons();

	void SetSpawnPoint();

	void OnPlayerStateInitialized();

private:

	UPROPERTY(VisibleAnywhere, Category = "Player Properties")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Player Properties")
	UCameraComponent* PlayerCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess="true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	/* Blast Components. */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Properties", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere)
	ULagCompensationComponent* LagCompensation;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType = EWeaponType::EWT_UnArmed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Properties", meta = (AllowPrivateAccess = "true"))
	uint32 PlayerWeapon = 9;*/

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;

	float InterpAO_Yaw;

	float AO_Pitch;

	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);

	/* Animation Montages. */

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;

	void HideCharacterIfCameraClose();

private:

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;

	float TurnThreshold = 0.5;

	FRotator ProxyRotationLastFrame;

	FRotator ProxyRotation;

	float ProxyYaw;

	float TimeSinceLastMovementReplication;

	float CalculateSpeed();

	/* Player Health. */

	UPROPERTY(EditAnywhere, Category = "Player Stats", meta = (AllowPrivateAccess="true"))
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats", meta = (AllowPrivateAccess="true"))
	float CurrentHealth = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/* Shield Health Implementation. */

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats", meta = (AllowPrivateAccess="true"))
	float CurrentShield = 0.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats", meta = (AllowPrivateAccess="true"))
	float MaxShield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	UPROPERTY()
	ABlastPlayerController* BlastPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	bool bLeftGame = false;

	/* Dissolve Effect. */

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	/* DynamicInstance that we can change at Runtime. */
	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	/* MaterialInstance, set on the Blueprint, used with the DynamicMaterialInstance. */
	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance;

	/* Team Colors. */

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* RedDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DefaultMaterial;

	/* Elim Effects. */

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

	UPROPERTY()
	ABlastPlayerState* BlastPlayerState;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownSystem;

	UPROPERTY()
	UNiagaraComponent* CrownComponent;

	/* Grenade Data. */

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/* Default Weapon. */

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	ABlastGameMode* BlastGameMode;

	/* Hit Boxes used for Server-Side Rewind. */

	UPROPERTY(EditAnywhere)
	UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

public:	

	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();
	
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }

	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }

	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return PlayerCamera; }

	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	FORCEINLINE bool IsElimmed() const { return bElimmed; }

	FORCEINLINE float GetHealth() const { return CurrentHealth; }

	FORCEINLINE float GetMaxHealth() const { return MaxHealth;  }

	FORCEINLINE float GetShield() const { return CurrentShield; }

	FORCEINLINE float GetMaxShield() const { return MaxShield; }

	FORCEINLINE void SetShield(float Amount) { CurrentShield = Amount; }

	ECombatState GetCombatState() const;

	//EWeaponType GetWeaponType() const;

	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }

	FORCEINLINE bool GetDisabledGameplay() const { return bDisableGameplay; }

	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }

	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }

	FORCEINLINE void SetHealth(float Amount) { CurrentHealth = Amount; }

	bool IsLocallyReloading();

	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }

	FORCEINLINE bool IsHoldingTheFlag() const;

	ETeam GetTeam();

	void SetHoldingTheFlag(bool bHolding);
};

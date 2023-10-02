// Fill out your copyright notice in the Description page of Project Settings.


#include "BlastCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "BlastAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "BlastComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiBlast/MultiBlast.h"
#include "PlayerController/BlastPlayerController.h"
#include "GameMode/BlastGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "MultiBlast/PlayerState/BlastPlayerState.h"
#include "BlastComponents/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BlastGameState.h"
#include "PlayerStart/TeamPlayerStart.h"
#include "BlastComponents/BuffComponent.h"
#include "TimerManager.h"
#include "Weapon/Weapon.h"
#include "Sound/SoundCue.h"

// Sets default values
ABlastCharacter::ABlastCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	PlayerCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment((RootComponent));

	Combat = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));	
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/* HitBoxes for Server-Side Rewind. */

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	//head->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	//pelvis->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	//spine_02->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	//spine_03->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	//upperarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	//upperarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	//lowerarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	//lowerarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	//hand_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	//hand_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	//backpack->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("backpack"), backpack);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	//blanket->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("backpack"), blanket);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	//thigh_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	//thigh_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);
	
	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	//calf_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	//calf_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	//foot_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	//foot_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlastCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlastCharacter, OverlappingWeapon, COND_OwnerOnly);

	DOREPLIFETIME(ABlastCharacter, CurrentHealth);

	DOREPLIFETIME(ABlastCharacter, CurrentShield);

	DOREPLIFETIME(ABlastCharacter, bDisableGameplay);
}

void ABlastCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}

	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}

	if (LagCompensation)
	{
		LagCompensation->Character = this;

		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlastPlayerController>(Controller);
		}
	}
}

void ABlastCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	
	TimeSinceLastMovementReplication = 0.f;
}

void ABlastCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	BlastGameMode = BlastGameMode == nullptr ? Cast<ABlastGameMode>(UGameplayStatics::GetGameMode(this)) : BlastGameMode;

	bool bMatchNotInProgress = BlastGameMode && BlastGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlastCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);

		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlastCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;

		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;

		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;

		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;

		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;

		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;

		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlastCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);

		//switch (Combat->EquippedWeapon->GetWeaponType())
		//{
		//case EWeaponType::EWT_AssaultRifle:
		//	SectionName = FName("Rifle");
		//	break;
		//}
	}
}

void ABlastCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlastCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlastCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapons();

	MulticastElim(bPlayerLeftGame);
}

//void ABlastCharacter::ShowSniperScopeWidget(bool bShowScope)
//{
//	bShowScope = true;
//}

void ABlastCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	if (BlastPlayerController)
	{
		BlastPlayerController->SetHUDWeaponAmmo(0);
	}

	bElimmed = true;
	PlayElimMontage();

	/* Initializing Dissolve Material Effect. */
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);

		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 220.f);
	}

	StartDissolve();

	/* Disable Character Component once Eliminated. */

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();

	/*if (Combat)
	{
		Combat->FireButtonPressed(false);
	}*/

	if (BlastPlayerController)
	{
		DisableInput(BlastPlayerController);
	}

	/* Disable Character Collision. */

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/* Spawn Elim Bot. */

	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);

		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}

	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}

	bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}

	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlastCharacter::ElimTimerFinished, ElimDelay);
}

void ABlastCharacter::ElimTimerFinished()
{
	BlastGameMode = BlastGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlastGameMode>() : BlastGameMode;

	if (BlastGameMode && !bLeftGame)
	{
		BlastGameMode->RequestRespawn(this, Controller);
	}

	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABlastCharacter::ServerLeaveGame_Implementation()
{
	BlastGameMode = BlastGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlastGameMode>() : BlastGameMode;

	BlastPlayerState = BlastPlayerState == nullptr ? GetPlayerState<ABlastPlayerState>() : BlastPlayerState;

	if (BlastGameMode && BlastPlayerState)
	{
		BlastGameMode->PlayerLeftGame(BlastPlayerState);
	}
}

void ABlastCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) { return; }

	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}

	/*if (Combat->EquippedWeapon->bDestroyWeapon)
	{
		Combat->EquippedWeapon->Destroy();
	}
	else
	{
		Combat->EquippedWeapon->Dropped();
	}*/
}

void ABlastCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}

		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}

		if (Combat->TheFlag)
		{
			Combat->TheFlag->Dropped();
		}
	}
}

void ABlastCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);

		FName SectionName("FromFront");
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlastCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	BlastGameMode = BlastGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlastGameMode>() : BlastGameMode;

	if (bElimmed ||  BlastGameMode == nullptr) { return; }

	Damage = BlastGameMode->CalculateDamage(InstigatorController, Controller, Damage);

	float DamageToHealth = Damage;

	if (CurrentShield > 0.f)
	{
		if (CurrentShield >= Damage)
		{
			CurrentShield = FMath::Clamp(CurrentShield - Damage, 0.f, MaxShield);

			DamageToHealth = 0.f;
		}

		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - CurrentShield, 0.f, Damage);
			CurrentShield = 0.f;
		}
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageToHealth, 0.f, MaxHealth);

	UpdateHUDHealth();

	UpdateHUDShield();

	PlayHitReactMontage();

	if (CurrentHealth == 0.f)
	{
		if (BlastGameMode)
		{
			BlastPlayerController = BlastPlayerController == nullptr ? Cast<ABlastPlayerController>(Controller) : BlastPlayerController;

			ABlastPlayerController* AttackerController = Cast<ABlastPlayerController>(InstigatorController);

			BlastGameMode->PlayerEliminated(this, BlastPlayerController, AttackerController);
		}
	}
	
}

void ABlastCharacter::PollInit()
{
	if (BlastPlayerState == nullptr)
	{
		BlastPlayerState = GetPlayerState<ABlastPlayerState>();

		if (BlastPlayerState)
		{
			OnPlayerStateInitialized();

			ABlastGameState* BlastGameState = Cast<ABlastGameState>(UGameplayStatics::GetGameState(this));

			if (BlastGameState && BlastGameState->TopScoringPlayers.Contains(BlastPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}


}

void ABlastCharacter::OnPlayerStateInitialized()
{
	BlastPlayerState->AddToScore(0.f);
	BlastPlayerState->AddToDefeats(0);

	SetTeamColor(BlastPlayerState->GetTeam());
	SetSpawnPoint();
}

void ABlastCharacter::SetSpawnPoint()
{
	if (HasAuthority() && BlastPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);

		TArray<ATeamPlayerStart*> TeamPlayerStarts;

		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == BlastPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}

		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];

			SetActorLocationAndRotation(ChosenPlayerStart->GetActorLocation(), ChosenPlayerStart->GetActorRotation());
		}
	}
}

void ABlastCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || DefaultMaterial == nullptr) { return; }

	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, DefaultMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;

	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;

	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
	}
}

// Called when the game starts or when spawned
void ABlastCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();

	UpdateHUDAmmo();
	
	UpdateHUDHealth();

	UpdateHUDShield();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlastCharacter::ReceiveDamage);
	}

	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

// Called every frame
void ABlastCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}*/

	RotateInPlace(DeltaTime);

	HideCharacterIfCameraClose();

	PollInit();
}

void ABlastCharacter::RotateInPlace(float DeltaTime)
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false;

		GetCharacterMovement()->bOrientRotationToMovement = true;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;

		return;
	}

	if (Combat && Combat->EquippedWeapon) GetCharacterMovement()->bOrientRotationToMovement = false;
	if (Combat && Combat->EquippedWeapon) bUseControllerRotationYaw = true;

	if (bDisableGameplay) 
	{ 
		bUseControllerRotationYaw = false;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;

		return; 
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}

	else
	{
		TimeSinceLastMovementReplication += DeltaTime;

		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}

		CalculateAO_Pitch();
	}
}

// Called to bind functionality to input
void ABlastCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ABlastCharacter::Jump);
	PlayerInputComponent->BindAction(FName("Equip"), IE_Pressed, this, &ABlastCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction(FName("Crouch"), IE_Pressed, this, &ABlastCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction(FName("Reload"), IE_Pressed, this, &ABlastCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction(FName("ThrowGrenade"), IE_Pressed, this, &ABlastCharacter::GrenadeButtonPressed);

	PlayerInputComponent->BindAction(FName("Aim"), IE_Pressed, this, &ABlastCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction(FName("Aim"), IE_Released, this, &ABlastCharacter::AimButtonReleased);

	PlayerInputComponent->BindAction(FName("Fire"), IE_Pressed, this, &ABlastCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(FName("Fire"), IE_Released, this, &ABlastCharacter::FireButtonReleased);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ABlastCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ABlastCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &ABlastCharacter::Turn);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &ABlastCharacter::LookUp);

}

void ABlastCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) { return; }

	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));

		AddMovementInput(Direction, Value);
	}
}

void ABlastCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) { return; }

	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

		AddMovementInput(Direction, Value);
	}
}

void ABlastCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlastCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlastCharacter::CrouchButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) { return; }

	if (bDisableGameplay) { return; }

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlastCharacter::ReloadButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) { return; }

	if (bDisableGameplay) { return; }

	if (Combat)
	{
		Combat->Reload();
	}
}

void ABlastCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) { return; }

	if (Combat)
	{
		if (Combat->bHoldingTheFlag) { return; }

		/*if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}*/

		if (Combat->CombatState == ECombatState::ECS_Unoccupied) ServerEquipButtonPressed();
		bool bSwap = Combat->ShouldSwapWeapons() && !HasAuthority() && Combat->CombatState == ECombatState::ECS_Unoccupied && OverlappingWeapon == nullptr;

		if (bSwap)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}

		//else
		//{
		//	
		//}
	}
}

void ABlastCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}

		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

void ABlastCharacter::AimButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) { return; }

	if (bDisableGameplay) { return; }

	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlastCharacter::AimButtonReleased()
{
	if (Combat && Combat->bHoldingTheFlag) { return; }

	if (bDisableGameplay) { return; }

	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlastCharacter::GrenadeButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) { return;}

	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlastCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) { return; }

	/* Locally Calculate Speed for the Character. */
	float Speed = CalculateSpeed();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // Standing Still & not Jumping.
	{
		bRotateRootBone = true;

		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);

		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);

		AO_Yaw = DeltaAimRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		bUseControllerRotationYaw = true;

		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir) // Running or Jumping.
	{
		bRotateRootBone = false;

		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);

		AO_Yaw = 0.f;

		bUseControllerRotationYaw = true;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
	/*if (HasAuthority() && !IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AO_Pitch: %f"), AO_Pitch);
	}*/
}

void ABlastCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		/* Map Pitch from the range [270, 360) to [-90, 0) */
		FVector2D InRange(270.f, 360.f);

		FVector2D OutRange(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlastCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) { return; }

	bRotateRootBone = false;

	float Speed = CalculateSpeed();

	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;

	ProxyRotation = GetActorRotation();

	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	//UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}

		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}

		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		
		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlastCharacter::Jump()
{
	if (Combat && Combat->bHoldingTheFlag) { return; }

	if (bDisableGameplay) { return; }

	if (bIsCrouched)
	{
		UnCrouch();
	}

	else
	{
		Super::Jump();
	}
}

void ABlastCharacter::FireButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) { return; }

	if (bDisableGameplay) { return; }

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlastCharacter::FireButtonReleased()
{
	if (Combat && Combat->bHoldingTheFlag) { return; }

	if (bDisableGameplay) { return; }

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlastCharacter::TurnInPlace(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("AO_Yaw: %f"), AO_Yaw);
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}

	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;

		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

/* Removed in Favour of using Variable Replication instead of an RPC. */
//void ABlastCharacter::MulticastHit_Implementation()
//{
//	PlayHitReactMontage();
//}

void ABlastCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) { return; }

	if ((PlayerCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}

		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}

	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}

		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ABlastCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();

	Velocity.Z = 0.f;

	return Velocity.Size();
}

void ABlastCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();

	if (CurrentHealth < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlastCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();

	if (CurrentShield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlastCharacter::UpdateHUDHealth()
{
	BlastPlayerController = BlastPlayerController == nullptr ? Cast<ABlastPlayerController>(Controller) : BlastPlayerController;

	if (BlastPlayerController)
	{
		BlastPlayerController->SetHUDHealth(CurrentHealth, MaxHealth);
	}
}

void ABlastCharacter::UpdateHUDShield()
{
	BlastPlayerController = BlastPlayerController == nullptr ? Cast<ABlastPlayerController>(Controller) : BlastPlayerController;

	if (BlastPlayerController)
	{
		BlastPlayerController->SetHUDShield(CurrentShield, MaxShield);
	}
}

void ABlastCharacter::UpdateHUDAmmo()
{
	BlastPlayerController = BlastPlayerController == nullptr ? Cast<ABlastPlayerController>(Controller) : BlastPlayerController;

	if (BlastPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlastPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlastPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ABlastCharacter::SpawnDefaultWeapon()
{
	BlastGameMode = BlastGameMode == nullptr ? Cast<ABlastGameMode>(UGameplayStatics::GetGameMode(this)) : BlastGameMode;
	UWorld* World = GetWorld();

	if (BlastGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;

		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

void ABlastCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) { return; }

	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(CrownSystem, GetMesh(), FName(), GetActorLocation() + FVector(0.f, 0.f, 110.f), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}

	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlastCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void ABlastCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlastCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlastCharacter::UpdateDissolveMaterial);

	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlastCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlastCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlastCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlastCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlastCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;

	return Combat->EquippedWeapon;
}

FVector ABlastCharacter::GetHitTarget() const
{
	if (Combat == nullptr) { return FVector(); }
	
	return Combat->HitTarget;
}

ECombatState ABlastCharacter::GetCombatState() const
{
	if (Combat == nullptr) { return ECombatState::ECS_MAX; }

	return Combat->CombatState;
}

bool ABlastCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) { return false; }

	return Combat->bLocallyReloading;
}

bool ABlastCharacter::IsHoldingTheFlag() const
{
	if (Combat == nullptr) { return false; }

	return Combat->bHoldingTheFlag;
}

ETeam ABlastCharacter::GetTeam()
{
	BlastPlayerState = BlastPlayerState == nullptr ? GetPlayerState<ABlastPlayerState>() : BlastPlayerState;
	if (BlastPlayerState == nullptr) { return ETeam::ET_NoTeam; }

	return BlastPlayerState->GetTeam();
}

void ABlastCharacter::SetHoldingTheFlag(bool bHolding)
{
	if (Combat == nullptr) { return; }

	Combat->bHoldingTheFlag = bHolding;
}

//EWeaponType ABlastCharacter::GetWeaponType() const
//{
//	if (Combat->EquippedWeapon == nullptr) { return EWeaponType::EWT_UnArmed; }
//
//	switch (WeaponType)
//	{
//	case EWeaponType::EWT_UnArmed:
//		WeaponType == 0;
//		break;
//
//	case EWeaponType::EWT_RocketLauncher:
//		PlayerWeapon = 1;
//		break;
//	}
//}






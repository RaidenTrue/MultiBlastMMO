// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlastPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

class ABlastHUD;
class UUserWidget;
class UMainMenu;
class UCharacterOverlay;
class ABlastGameState;
class ABlastPlayerState;
class ABlastGameMode;

/**
 * 
 */
UCLASS()
class MULTIBLAST_API ABlastPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void SetHUDHealth(float Health, float MaxHealth);

	void SetHUDShield(float Shield, float MaxShield);

	void SetHUDScore(float Score);

	void SetHUDDefeats(int32 Defeats);

	void SetHUDWeaponAmmo(int32 Ammo);

	void SetHUDCarriedAmmo(int32 Ammo);

	void SetHUDMatchCountdown(float CountdownTime);

	void SetHUDAnnouncementCountdown(float CountdownTime);

	void SetHUDGrenades(int32 Grenades);
	
	virtual void OnPossess(APawn* InPawn) override;

	virtual float GetServerTime(); // Synced with Server World Clock.

	void OnMatchStateSet(FName State, bool bTeamsMatch = false);

	void HandleMatchHasStarted(bool bTeamsMatch = false);

	void HandleCooldown();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

	void HideTeamScores();

	void InitTeamScores();

	void SetHUDRedTeamScore(int32 RedScore);

	void SetHUDBlueTeamScore(int32 BlueScore);

protected:

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	virtual void Tick(float DeltaTime) override;

	virtual void ReceivedPlayer() override; // Synced with Server Clock, as soon as Possible.

	void SetHUDTime();

	void PollInit();

	/* Sync Time between Client & Server. */

	// Requests the Current Server Time, passing in the Clients Time when the Request was sent. 
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the Current Server time to the Client, in response to Server Request, Server Time. 
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// Difference between Client & Server Time.
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = "Sync Server Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	void HighPingWarning();

	void StopHighPingWarning();

	void CheckPing(float DeltaTime);

	void ShowReturnToMainMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<ABlastPlayerState*>& Players);

	FString GetTeamsInfoText(ABlastGameState* BlastGameState);

private:

	UPROPERTY()
	ABlastHUD* BlastHUD;

	/* MainMenu - Quit Game. */

	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	UMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;
  
	UPROPERTY()
	ABlastGameMode* BlastGameMode;

	float MatchTime = 0.f;

	float WarmupTime = 0.f;

	float CooldownTime = 0.f;

	float LevelStartingTime = 0.f;

	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	/* No Longer in Use. Replaced with Multiple Bools for each HUD Element. (Health, Shields, Score etc. */
	//bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	bool bInitializeHealth = false;

	float HUDMaxHealth;

	float HUDScore;
	bool bInitializeScore = false;

	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	float HUDShield;
	bool bInitializeShield = false;

	float HUDMaxShield;

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;

	UPROPERTY(EditAnywhere)
	int32 HighPingWarningLoops;
};

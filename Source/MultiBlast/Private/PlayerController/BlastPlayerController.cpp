// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/BlastPlayerController.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/Announcment.h"
#include "HUD/MainMenu.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameMode/BlastGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "BlastComponents/CombatComponent.h"
#include "BlastGameState.h"
#include "BlastCharacter.h"
#include "MultiBlast/PlayerState/BlastPlayerState.h"
#include "MultiBlast/BlastTypes/Announcement.h"
#include "Net/UnrealNetwork.h"
#include "HUD/BlastHUD.h"

void ABlastPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->HealthBar && BlastHUD->CharacterOverlay->HealthText;

	/*if (BlastHUD) UE_LOG(LogTemp, Warning, TEXT("BlastHUD Valid"));
	if (BlastHUD && BlastHUD->CharacterOverlay) UE_LOG(LogTemp, Warning, TEXT("BlastHUD & CharacterOverlay Valid."))*/

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;

		BlastHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlastHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}

	else
	{
		bInitializeHealth = true;

		HUDHealth = Health;

		HUDMaxHealth = MaxHealth;
	}
}

void ABlastPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->ShieldBar && BlastHUD->CharacterOverlay->ShieldText;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;

		BlastHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);

		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlastHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}

	else
	{
		bInitializeShield = true;

		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlastPlayerController::SetHUDScore(float Score)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlastHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}

	else
	{
		bInitializeScore = true;

		HUDScore = Score;
	}
}

void ABlastPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->DefeatsAmount;

	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlastHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}

	else
	{
		bInitializeDefeats = true;

		HUDDefeats = Defeats;
	}
}

void ABlastPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString WeaponAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlastHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(WeaponAmmoText));
	}

	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlastPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlastHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}

	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlastPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlastHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60.f;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlastHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlastPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->Announcement && BlastHUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlastHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60.f;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlastHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlastPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && CharacterOverlay->GrenadesText;

	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlastHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}

	else
	{
		bInitializeGrenades = true;

		HUDGrenades = Grenades;
	}
}

void ABlastPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ABlastPlayerController::HideTeamScores()
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->RedTeamScore && BlastHUD->CharacterOverlay->BlueTeamScore && BlastHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		BlastHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		BlastHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		BlastHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}

void ABlastPlayerController::InitTeamScores()
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->RedTeamScore && BlastHUD->CharacterOverlay->BlueTeamScore && BlastHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");

		BlastHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		BlastHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		BlastHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}

void ABlastPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);

		BlastHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ABlastPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->BlueTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);

		BlastHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ABlastPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();

	if (Attacker && Victim && Self)
	{
		BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;
		if (BlastHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				BlastHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				BlastHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				BlastHUD->AddElimAnnouncement("You", "Your-Self");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				BlastHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Them-Selfs");
				return;
			}
			BlastHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void ABlastPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlastHUD = Cast<ABlastHUD>(GetHUD());

	ServerCheckMatchState();
}

void ABlastPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr) { return; }

	InputComponent->BindAction("QuitGame", IE_Pressed, this, &ABlastPlayerController::ShowReturnToMainMenu);
}

void ABlastPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlastPlayerController, MatchState);

	DOREPLIFETIME(ABlastPlayerController, bShowTeamScores);
}

void ABlastPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);

	PollInit();

	CheckPing(DeltaTime);
}

void ABlastPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;

	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;

		if (PlayerState)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Player PING is: %d"), PlayerState->GetPing() * 4);
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold) // Ping is Compressed. It's actually Ping Divided by 4.
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}

		HighPingRunningTime = 0.f;
	}

	bool bHighPingAnimation = BlastHUD &&
		BlastHUD->CharacterOverlay &&
		BlastHUD->CharacterOverlay->HighPingAnimation &&
		BlastHUD->CharacterOverlay->IsAnimationPlaying(BlastHUD->CharacterOverlay->HighPingAnimation);

	if (bHighPingAnimation)
	{
		PingAnimationRunningTime += DeltaTime;

		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlastPlayerController::ShowReturnToMainMenu()
{
	// Show the Return to Main Menu Widget.
	if (ReturnToMainMenuWidget == nullptr) { return; }

	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void ABlastPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

/* Is the PING too High? */
void ABlastPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABlastPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;

	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlastPlayerController::HighPingWarning()
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->HighPingImage && BlastHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		BlastHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlastHUD->CharacterOverlay->PlayAnimation(BlastHUD->CharacterOverlay->HighPingAnimation, 0.f, HighPingWarningLoops);
	}
}

void ABlastPlayerController::StopHighPingWarning()
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	bool bHUDValid = BlastHUD && BlastHUD->CharacterOverlay && BlastHUD->CharacterOverlay->HighPingImage && BlastHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		BlastHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (BlastHUD->CharacterOverlay->IsAnimationPlaying(BlastHUD->CharacterOverlay->HighPingAnimation))
		{
			BlastHUD->CharacterOverlay->StopAnimation(BlastHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ABlastPlayerController::ServerCheckMatchState_Implementation()
{
	ABlastGameMode* GameMode = Cast<ABlastGameMode>(UGameplayStatics::GetGameMode(this));

	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;

		MatchTime = GameMode->MatchTime;

		CooldownTime = GameMode->CooldownTime;

		LevelStartingTime = GameMode->LevelStartingTime;

		MatchState = GameMode->GetMatchState();

		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void ABlastPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;

	MatchTime = Match;

	CooldownTime = Cooldown;

	LevelStartingTime = StartingTime;

	MatchState = StateOfMatch;

	OnMatchStateSet(MatchState);

	if (BlastHUD && MatchState == MatchState::WaitingToStart)
		{
			BlastHUD->AddAnnouncement();
		}
}

void ABlastPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlastCharacter* BlastCharacter = Cast<ABlastCharacter>(InPawn);

	if (BlastCharacter)
	{
		SetHUDHealth(BlastCharacter->GetHealth(), BlastCharacter->GetMaxHealth());
	}
}

void ABlastPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}

	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlastPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}

	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlastPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority()) { bShowTeamScores = bTeamsMatch; }

	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	if (BlastHUD)
	{
		if (BlastHUD->CharacterOverlay == nullptr) BlastHUD->AddCharacterOverlay();

		if (BlastHUD->Announcement)
		{
			BlastHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}

		if (!HasAuthority()) { return; }

		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void ABlastPlayerController::HandleCooldown()
{
	BlastHUD = BlastHUD == nullptr ? Cast<ABlastHUD>(GetHUD()) : BlastHUD;

	if (BlastHUD)
	{
		BlastHUD->CharacterOverlay->RemoveFromParent();
			
		bool bHUDValid = BlastHUD->Announcement && BlastHUD->Announcement->AnnouncementText && BlastHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			BlastHUD->Announcement->SetVisibility(ESlateVisibility::Visible);

			FString AnnouncmentText = Announcement::NewMatchStartsIn;

			BlastHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncmentText));

			ABlastGameState* BlastGameState = Cast<ABlastGameState>(UGameplayStatics::GetGameState(this));

			ABlastPlayerState* BlastPlayerState = GetPlayerState<ABlastPlayerState>();

			if (BlastGameState && BlastPlayerState)
			{
				TArray<ABlastPlayerState*> TopPlayers = BlastGameState->TopScoringPlayers;

				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(BlastGameState) : GetInfoText(TopPlayers);

				BlastHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}

	ABlastCharacter* BlastCharacter = Cast<ABlastCharacter>(GetPawn());

	if (BlastCharacter && BlastCharacter->GetCombat())
	{
		BlastCharacter->bDisableGameplay = true;

		BlastCharacter->GetCombat()->FireButtonPressed(false);
	}
}

FString ABlastPlayerController::GetInfoText(const TArray<ABlastPlayerState*>& Players)
{
	ABlastPlayerState* BlastPlayerState = GetPlayerState<ABlastPlayerState>();

	if (BlastPlayerState == nullptr) { return FString(); }

	FString InfoTextString;

	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}

	else if (Players.Num() == 1 && Players[0] == BlastPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}

	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}

	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString ABlastPlayerController::GetTeamsInfoText(ABlastGameState* BlastGameState)
{
	if (BlastGameState == nullptr) { return FString(); }
	
	FString InfoTextString;

	const int32 RedTeamScore = BlastGameState->RedTeamScore;
	const int32 BlueTeamScore = BlastGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);

		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));

		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;

		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (RedTeamScore < BlueTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;

		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}

void ABlastPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;

	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;

	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	/* Currently this is Bugging, as the Server "CooldownTimer does not Update. Needs a Fix. */
	/*if (HasAuthority())
	{
		BlastGameMode = BlastGameMode == nullptr ? Cast<ABlastGameMode>(UGameplayStatics::GetGameMode(this)) : BlastGameMode;

		if (BlastGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlastGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}*/

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}

		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	
	CountdownInt = SecondsLeft;
}

void ABlastPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlastHUD && BlastHUD->CharacterOverlay)
		{
			CharacterOverlay = BlastHUD->CharacterOverlay;

			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);

				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);

				if (bInitializeScore) SetHUDScore(HUDScore);

				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);

				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);

				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				ABlastCharacter* BlastCharacter = Cast<ABlastCharacter>(GetPawn());
				if (BlastCharacter && BlastCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(BlastCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void ABlastPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();

	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlastPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;

	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlastPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlastPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

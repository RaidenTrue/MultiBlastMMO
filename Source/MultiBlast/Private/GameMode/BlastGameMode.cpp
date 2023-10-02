// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BlastGameMode.h"
#include "PlayerController/BlastPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "MultiBlast/PlayerState/BlastPlayerState.h"
#include "BlastGameState.h"
#include "BlastCharacter.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlastGameMode::ABlastGameMode()
{
	bDelayedStart = true;
}

void ABlastGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlastGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}

	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}

	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlastGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlastPlayerController* BlastPlayer = Cast<ABlastPlayerController>(*It);

		if (BlastPlayer)
		{
			BlastPlayer->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

float ABlastGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{


	return BaseDamage;
}

void ABlastGameMode::PlayerEliminated(ABlastCharacter* ElimmedCharacter, ABlastPlayerController* VictimController, ABlastPlayerController* AttackerController)
{
	ABlastPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlastPlayerState>(AttackerController->PlayerState) : nullptr;

	ABlastPlayerState* VictimPlayerState = VictimController ? Cast<ABlastPlayerState>(VictimController->PlayerState) : nullptr;

	ABlastGameState* BlastGameState = GetGameState<ABlastGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlastGameState)
	{
		TArray<ABlastPlayerState*> PlayersCurrentlyInTheLead;

		for (auto LeadPlayer : BlastGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		AttackerPlayerState->AddToScore(1.f);

		BlastGameState->UpdateTopScore(AttackerPlayerState);

		if (BlastGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlastCharacter* Leader = Cast<ABlastCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!BlastGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ABlastCharacter* Green = Cast<ABlastCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Green)
				{
					Green->MulticastLostTheLead();
				}
			}
		}
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ABlastPlayerController* BlastPlayer = Cast<ABlastPlayerController>(*Iterator);

		if (BlastPlayer && AttackerPlayerState && VictimPlayerState)
		{
			BlastPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void ABlastGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}

	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;

		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);

		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ABlastGameMode::PlayerLeftGame(ABlastPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) { return; }

	ABlastGameState* BlastGameState = GetGameState<ABlastGameState>();

	if (BlastGameState && BlastGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlastGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}

	ABlastCharacter* CharacterLeaving = Cast<ABlastCharacter>(PlayerLeaving->GetPawn());

	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}



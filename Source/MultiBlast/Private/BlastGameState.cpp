// Fill out your copyright notice in the Description page of Project Settings.


#include "BlastGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/BlastPlayerController.h"
#include "MultiBlast/PlayerState/BlastPlayerState.h"

void ABlastGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlastGameState, TopScoringPlayers);

	DOREPLIFETIME(ABlastGameState, RedTeamScore);

	DOREPLIFETIME(ABlastGameState, BlueTeamScore);
}

void ABlastGameState::UpdateTopScore(ABlastPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);

		TopScore = ScoringPlayer->GetScore();
	}

	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}

	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();

		TopScoringPlayers.AddUnique(ScoringPlayer);

		TopScore = ScoringPlayer->GetScore();
	}
}

void ABlastGameState::RedTeamScores()
{
	++RedTeamScore;

	ABlastPlayerController* BPlayer = Cast<ABlastPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlastGameState::BlueTeamScores()
{
	++BlueTeamScore;

	ABlastPlayerController* BPlayer = Cast<ABlastPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlastGameState::OnRep_RedTeamScore()
{
	ABlastPlayerController* BPlayer = Cast<ABlastPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlastGameState::OnRep_BlueTeamScore()
{
	ABlastPlayerController* BPlayer = Cast<ABlastPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/CaptureTheFlagGameMode.h"
#include "Weapon/Flag.h"
#include "BlastGameState.h"
#include "CaptureTheFlag/FlagZone.h"
#include "PlayerController/BlastPlayerController.h"
#include "BlastCharacter.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlastCharacter* ElimmedCharacter, ABlastPlayerController* VictimController, ABlastPlayerController* AttackerController)
{
	ABlastGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);


}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;

	ABlastGameState* BGameState = Cast<ABlastGameState>(GameState);
	if (BGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}

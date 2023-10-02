// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TeamsGameMode.h"
#include "MultiBlast/PlayerState/BlastPlayerState.h"
#include "PlayerController/BlastPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "BlastGameState.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlastGameState* BGameState = Cast<ABlastGameState>(UGameplayStatics::GetGameState(this));

	if (BGameState)
	{
		ABlastPlayerState* BPState = NewPlayer->GetPlayerState<ABlastPlayerState>();

		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	ABlastGameState* BGameState = Cast<ABlastGameState>(UGameplayStatics::GetGameState(this));

	ABlastPlayerState* BPState = Exiting->GetPlayerState<ABlastPlayerState>();

	if (BGameState && BPState)
	{
		if (BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlastGameState* BGameState = Cast<ABlastGameState>(UGameplayStatics::GetGameState(this));

	if (BGameState)
	{
		for (auto PState : BGameState->PlayerArray)
		{
			ABlastPlayerState* BPState = Cast<ABlastPlayerState>(PState.Get());

			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ABlastPlayerState* AttackerPState = Attacker->GetPlayerState<ABlastPlayerState>();
	ABlastPlayerState* VictimPState = Victim->GetPlayerState<ABlastPlayerState>();

	if (AttackerPState == nullptr || VictimPState == nullptr) { return BaseDamage; }

	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}

	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.0f;
	}

	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(ABlastCharacter* ElimmedCharacter, ABlastPlayerController* VictimController, ABlastPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	ABlastGameState* BGameState = Cast<ABlastGameState>(UGameplayStatics::GetGameState(this));
	ABlastPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlastPlayerState>(AttackerController->PlayerState) : nullptr;

	if (BGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}

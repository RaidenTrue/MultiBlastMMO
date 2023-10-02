// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiBlast/PlayerState/BlastPlayerState.h"
#include "MultiBlast/Public/BlastCharacter.h"
#include "PlayerController/BlastPlayerController.h"
#include "Net/UnrealNetwork.h"


void ABlastPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlastPlayerState, Defeats);

	DOREPLIFETIME(ABlastPlayerState, Team);
}

void ABlastPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ABlastCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlastPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlastPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	Character = Character == nullptr ? Cast<ABlastCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlastPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlastPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;

	Character = Character == nullptr ? Cast<ABlastCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlastPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlastPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlastCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlastPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlastPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	ABlastCharacter* BCharacter = Cast<ABlastCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

void ABlastPlayerState::OnRep_Team()
{
	ABlastCharacter* BCharacter = Cast<ABlastCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}


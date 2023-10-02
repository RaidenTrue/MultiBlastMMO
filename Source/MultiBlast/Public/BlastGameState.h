// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlastGameState.generated.h"

class ABlastPlayerState;

/**
 * 
 */
UCLASS()
class MULTIBLAST_API ABlastGameState : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(ABlastPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ABlastPlayerState*> TopScoringPlayers;

	/* Teams. */

	void RedTeamScores();

	void BlueTeamScores();

	TArray<ABlastPlayerState*> RedTeam;

	TArray<ABlastPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_BlueTeamScore();

private:

	float TopScore = 0.f;
	
};

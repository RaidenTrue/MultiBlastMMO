// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiBlast\BlastTypes\Team.h"
#include "GameFramework\PlayerState.h"
#include "BlastPlayerState.generated.h"

class ABlastCharacter;
class ABlastPlayerController;

/**
 * 
 */
UCLASS()
class MULTIBLAST_API ABlastPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* Replication Notifies. */
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(float ScoreAmount);

	void AddToDefeats(int32 DefeatsAmount);

private:

	UPROPERTY()
	ABlastCharacter* Character;

	UPROPERTY()
	ABlastPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();

public:

	FORCEINLINE ETeam GetTeam() const { return Team; }

	void SetTeam(ETeam TeamToSet);
};

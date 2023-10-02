// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/BlastGameMode.h"
#include "TeamsGameMode.generated.h"

class ABlastCharacter;
class ABlastPlayerController;

/**
 * 
 */
UCLASS()
class MULTIBLAST_API ATeamsGameMode : public ABlastGameMode
{
	GENERATED_BODY()

public:

	ATeamsGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;

	virtual void PlayerEliminated(ABlastCharacter* ElimmedCharacter, ABlastPlayerController* VictimController, ABlastPlayerController* AttackerController) override;

protected:

	virtual void HandleMatchHasStarted() override;
	
};

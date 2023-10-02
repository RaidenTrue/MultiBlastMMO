// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

class AFlag;
class AFlagZone;
class ABlastCharacter;
class ABlastPlayerController;

/**
 * 
 */
UCLASS()
class MULTIBLAST_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:

	virtual void PlayerEliminated(ABlastCharacter* ElimmedCharacter, ABlastPlayerController* VictimController, ABlastPlayerController* AttackerController) override;

	void FlagCaptured(AFlag* Flag, AFlagZone* Zone);
	
};

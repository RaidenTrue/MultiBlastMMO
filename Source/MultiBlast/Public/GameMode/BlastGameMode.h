// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlastGameMode.generated.h"

class ABlastPlayerState;

namespace MatchState
{
	extern MULTIBLAST_API const FName Cooldown; // Match duration has been reached, Display Winner & Begin Cooldown Timer.
}

class ABlastCharacter;
class ABlastPlayerController;
/**
 * 
 */
UCLASS()
class MULTIBLAST_API ABlastGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	ABlastGameMode();

	virtual void Tick(float DeltaTime) override;

	virtual void PlayerEliminated(ABlastCharacter* ElimmedCharacter, ABlastPlayerController* VictimController, ABlastPlayerController* AttackerController);
	
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);

	void PlayerLeftGame(ABlastPlayerState* PlayerLeaving);

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 5.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 60.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 5.f;

	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;

protected:

	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:

	float CountdownTime = 0.f;

public:

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};

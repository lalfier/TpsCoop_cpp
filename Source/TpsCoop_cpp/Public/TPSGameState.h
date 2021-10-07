// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TPSGameState.generated.h"


UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,
	WaveInProgress,
	WaitingToComplete,	// No longer spawning new bots, waiting for players to kill remaining bots
	WaveComplete,
	GameOver
};


/**
 * We use GameState class to replicate info from GameMode to clients.
 */
UCLASS()
class TPSCOOP_CPP_API ATPSGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	void SetWaveState(EWaveState NewState);
	
protected:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;

	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

};

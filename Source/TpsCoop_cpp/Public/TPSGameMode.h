// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPSGameMode.generated.h"

// Forward declaration
enum class EWaveState : uint8;


// VICTIM actor, KILLER actor
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController);


/**
* Game mode only exists on server. We use GameState class to replicate info from GameMode to clients.
* Player controller only exists on local client and server. We use PlayerState to replicate info from PlayerController to other clients.
*/
UCLASS()
class TPSCOOP_CPP_API ATPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Sets default values for this GameMode's properties
	ATPSGameMode();

	virtual void StartPlay() override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;
	
protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FTimerHandle TimerHandle_BotSpawner;

	FTimerHandle TimerHandle_NextWaveStart;

	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

	// Bots to spawn in current wave
	int32 NrOfBotsToSpawn;

	// Hook for BP to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	// Start Spawning Bots
	void StartWave();

	// Stop Spawning Bots
	void EndWave();

	// Set timer for next StartWave
	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	void RestartDeadPlayers();

};

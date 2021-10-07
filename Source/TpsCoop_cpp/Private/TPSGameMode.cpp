// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSGameMode.h"
#include "TPSGameState.h"
#include "TPSPlayerState.h"
#include "EngineUtils.h"
#include "Components/TPSHealthComponent.h"


// Sets default values
ATPSGameMode::ATPSGameMode()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0;	// Limit tick once per second

	GameStateClass = ATPSGameState::StaticClass();
	PlayerStateClass = ATPSPlayerState::StaticClass();

	TimeBetweenWaves = 2.0f;
}

void ATPSGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ATPSGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ATPSGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);

	RestartDeadPlayers();
}

void ATPSGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if(NrOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		// Exit if we are still spawning bots or already preparing
		return;
	}

	bool bIsAnyBotAlive = false;
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* BotPawn = *It;
		if(BotPawn == nullptr || BotPawn->IsPlayerControlled())
		{
			// Not Bot/AI pawn
			continue;
		}

		UTPSHealthComponent* HealthComp = Cast<UTPSHealthComponent>(BotPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));
		if(HealthComp && HealthComp->GetCurrentHealth() > 0.0f)
		{
			// Still alive bots, exit
			bIsAnyBotAlive = true;
			break;
		}
	}

	if(!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);

		PrepareForNextWave();
	}
}

void ATPSGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if(PC)
		{
			APawn* PlayerPawn = PC->GetPawn();
			if(PlayerPawn)
			{
				UTPSHealthComponent* HealthComp = Cast<UTPSHealthComponent>(PlayerPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));
				if(ensure(HealthComp) && HealthComp->GetCurrentHealth() > 0.0f)
				{
					// Still alive players, exit
					return;
				}
			}
		}
	}

	// No player alive
	GameOver();
}

void ATPSGameMode::GameOver()
{
	EndWave();

	// TODO: Finish up the match, present 'Game Over' to players

	SetWaveState(EWaveState::GameOver);

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! All players dead!"));
}

void ATPSGameMode::SetWaveState(EWaveState NewState)
{
	ATPSGameState* GS = GetGameState<ATPSGameState>();
	if(ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void ATPSGameMode::RestartDeadPlayers()
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if(PC)
		{
			APawn* PlayerPawn = PC->GetPawn();
			if(PlayerPawn == nullptr)
			{
				RestartPlayer(PC);
			}
		}
	}
}

void ATPSGameMode::StartWave()
{
	WaveCount++;
	NrOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ATPSGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);

	SetWaveState(EWaveState::WaveInProgress);
}

void ATPSGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);
}

void ATPSGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();
	NrOfBotsToSpawn--;

	if(NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}

void ATPSGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckWaveState();
	CheckAnyPlayerAlive();
}

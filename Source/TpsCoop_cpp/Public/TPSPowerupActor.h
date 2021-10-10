// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSPowerupActor.generated.h"

// Forward declaration
class USoundCue;


UCLASS()
class TPSCOOP_CPP_API ATPSPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATPSPowerupActor();

protected:
	// Time between power-up ticks
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	// Total times we apply the power-up effect
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNrOfTicks;

	// Total number of ticks applied
	int32 TicksProcessed;

	FTimerHandle TimerHandle_PowerupTick;

	// Keeps state of the power-up
	UPROPERTY(ReplicatedUsing=OnRep_PowerupActive)
	bool bIsPowerupActive;

	// Actor on which power-up is activated
	UPROPERTY(Replicated)
	AActor* ActivateForPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundCue* ActivateSound;

	UFUNCTION()
	void OnTickPowerup();

	UFUNCTION()
	void OnRep_PowerupActive();

	void OnPowerupStateChaged();

public:

	void ActivatePowerup(AActor* ActivateFor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated(AActor* ActivateFor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();

};

// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPowerupActor.h"


// Sets default values
ATPSPowerupActor::ATPSPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;
}

// Called when the game starts or when spawned
void ATPSPowerupActor::BeginPlay()
{
	Super::BeginPlay();	

}

void ATPSPowerupActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	// Last tick
	if(TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		// Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ATPSPowerupActor::ActivatePowerup()
{
	if(PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ATPSPowerupActor::OnTickPowerup, PowerupInterval, true, 0.0f);
	}
	else
	{
		OnTickPowerup();
	}
}

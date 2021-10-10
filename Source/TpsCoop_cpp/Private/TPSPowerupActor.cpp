// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPowerupActor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ATPSPowerupActor::ATPSPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;
	bIsPowerupActive = false;

	bReplicates = true;
}

void ATPSPowerupActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	// Last tick
	if(TicksProcessed >= TotalNrOfTicks)
	{
		bIsPowerupActive = false;
		OnPowerupStateChaged();

		// Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ATPSPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChaged();
}

void ATPSPowerupActor::OnPowerupStateChaged()
{
	if(bIsPowerupActive)
	{
		OnActivated(ActivateForPlayer);
		// Play sound
		UGameplayStatics::PlaySoundAtLocation(this, ActivateSound, GetActorLocation());
	}
	else
	{
		OnExpired();
	}	
}

void ATPSPowerupActor::ActivatePowerup(AActor* ActivateFor)
{
	ActivateForPlayer = ActivateFor;
	bIsPowerupActive = true;
	OnPowerupStateChaged();

	if(PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ATPSPowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		OnTickPowerup();
	}
}

// Apply rules for variable replications.
void ATPSPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This macro is default: replicate bIsPowerupActive, ActivateForPlayer variable to all clients connected.
	DOREPLIFETIME(ATPSPowerupActor, bIsPowerupActive);
	DOREPLIFETIME(ATPSPowerupActor, ActivateForPlayer);
}

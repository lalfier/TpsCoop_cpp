// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSGameState.h"
#include "Net/UnrealNetwork.h"


void ATPSGameState::SetWaveState(EWaveState NewState)
{
	if(HasAuthority())
	{
		EWaveState OldState = WaveState;
		WaveState = NewState;
		WaveStateChanged(NewState, OldState);
	}
}

void ATPSGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}

// Apply rules for variable replications.
void ATPSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This macro is default: replicate WaveState variable to all clients connected.
	DOREPLIFETIME(ATPSGameState, WaveState);
}

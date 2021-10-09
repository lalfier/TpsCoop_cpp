// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayerState.h"
#include "Net/UnrealNetwork.h"


void ATPSPlayerState::AddScore(float ScoreDelta)
{
	SetScore(GetScore() + ScoreDelta);

	FString PlayerName = GetPlayerName();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("%s: %.0f"), *PlayerName, GetScore()));
}

void ATPSPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	FString PlayerName = GetPlayerName();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("%s: %.0f"), *PlayerName, GetScore()));
}

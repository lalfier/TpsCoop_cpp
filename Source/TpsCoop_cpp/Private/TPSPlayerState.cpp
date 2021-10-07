// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayerState.h"


void ATPSPlayerState::AddScore(float ScoreDelta)
{
	SetScore(GetScore() + ScoreDelta);
}

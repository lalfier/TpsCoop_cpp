// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TPSPlayerState.generated.h"


/**
 * We use PlayerState to replicate info from PlayerController to other clients.
 */
UCLASS()
class TPSCOOP_CPP_API ATPSPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void AddScore(float SocreDelta);

};

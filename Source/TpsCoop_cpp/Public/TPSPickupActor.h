// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSPickupActor.generated.h"

// Forward declaration
class UDecalComponent;
class USphereComponent;
class ATPSPowerupActor;


UCLASS()
class TPSCOOP_CPP_API ATPSPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATPSPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<ATPSPowerupActor> PowerupClass;

	ATPSPowerupActor* PowerupInstance;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	float CooldownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

	void RespawnPowerup();

public:

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};

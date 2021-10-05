// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPickupActor.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
ATPSPickupActor::ATPSPickupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64.0f, 75.0f, 75.0f);
	DecalComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATPSPickupActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATPSPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// TODO: Grant a power-up to player if available
}

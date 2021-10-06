// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPickupActor.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "TPSPowerupActor.h"


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

	CooldownDuration = 10.0f;

	bReplicates = true;
}

// Called when the game starts or when spawned
void ATPSPickupActor::BeginPlay()
{
	Super::BeginPlay();
	
	if(HasAuthority())
	{
		RespawnPowerup();
	}	
}

void ATPSPickupActor::RespawnPowerup()
{
	if(PowerupClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerupClass is nullptr in %s. Please update your Blueprint."), *GetName());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PowerupInstance = GetWorld()->SpawnActor<ATPSPowerupActor>(PowerupClass, GetTransform(), SpawnParams);
}

void ATPSPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// Grant a power-up to player if available
	if(HasAuthority() && PowerupInstance)
	{
		PowerupInstance->ActivatePowerup(OtherActor);
		PowerupInstance = nullptr;

		// Set timer to re-spawn
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ATPSPickupActor::RespawnPowerup, CooldownDuration);
	}
}

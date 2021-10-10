// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSExplosiveBarrel.h"
#include "Net/UnrealNetwork.h"
#include "Components/TPSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "DrawDebugHelpers.h"


// Console variable
static int32 DrawBarrelDebugLinesValue = 0;
FAutoConsoleVariableRef DrawBarrelDebugDebugLines(TEXT("COOP.DebugBarrel"), DrawBarrelDebugLinesValue, TEXT("Draw Debug Lines for Barrel"), ECVF_Cheat);


// Sets default values
ATPSExplosiveBarrel::ATPSExplosiveBarrel()
{
	HealthComp = CreateDefaultSubobject<UTPSHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ATPSExplosiveBarrel::OnHealthChanged);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	// Set physics body to let radial component affect us (when a nearby barrel explodes)
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetCanEverAffectNavigation(false);
	RootComponent = MeshComp;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 400.0f;
	RadialForceComp->ImpulseStrength = 600.0f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;	// Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true;	// Ignore self

	ExplosionDamage = 100.0f;
	ExplosionRadius = 400.0f;

	bReplicates = true;
	SetReplicateMovement(true);
}

void ATPSExplosiveBarrel::OnHealthChanged(UTPSHealthComponent* InHealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if(bExploded)
	{
		return;
	}

	if(CurrentHealth <= 0.0f)
	{
		// Explode!
		bExploded = true;

		// Play FX and change self material
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		// Override material on mesh with blackened version
		MeshComp->SetMaterial(0, ExplodedMaterial);
		// Play sound
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

		// Boost the barrel upwards
		FVector BoostIntensity = FVector::UpVector * RadialForceComp->ImpulseStrength;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		// Blast away nearby physics actors
		RadialForceComp->FireImpulse();

		// Apply radial damage to nearby actors
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, InstigatedBy, false, ECC_Visibility);
		if(DrawBarrelDebugLinesValue > 0)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Yellow, false, 1.0f, 0, 2.0f);
		}		
	}
}

void ATPSExplosiveBarrel::OnRep_Exploded()
{
	// Play FX and change self material
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	// Override material on mesh with blackened version
	MeshComp->SetMaterial(0, ExplodedMaterial);
	// Play sound
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
}

// Apply rules for variable replications.
void ATPSExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This macro is default: replicate bExploded variable to all clients connected.
	DOREPLIFETIME(ATPSExplosiveBarrel, bExploded);
}

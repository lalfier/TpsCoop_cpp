// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSExplosiveBarrel.h"
#include "Components/TPSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "DrawDebugHelpers.h"


// Sets default values
ATPSExplosiveBarrel::ATPSExplosiveBarrel()
{
	HealthComp = CreateDefaultSubobject<UTPSHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ATPSExplosiveBarrel::OnHealthChanged);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	// Set physics body to let radial component affect us (when a nearby barrel explodes)
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250.0f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;	// Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true;	// Ignore self

	ExplosionImpulse = 400;
	ExplosionDamage = 100;
	ExplosionRadius = 300;
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

		// Boost the barrel upwards
		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		// Play FX and change self material
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		// Override material on mesh with blackened version
		MeshComp->SetMaterial(0, ExplodedMaterial);

		// Blast away nearby physics actors
		RadialForceComp->FireImpulse();

		// Apply radial damage to nearby actors
		TArray<AActor*> IgnoredActors;
		UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, UDamageType::StaticClass(), IgnoredActors, this, nullptr, false, ECC_Visibility);
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Yellow, false, 1.0f, 0, 2.0f);
	}
}

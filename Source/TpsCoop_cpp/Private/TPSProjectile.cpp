// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/TPSHealthComponent.h"


// Sets default values
ATPSProjectile::ATPSProjectile()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	// Set physics body to let radial component affect us (when a nearby barrel explodes)
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetNotifyRigidBodyCollision(true);
	// Event called when component hits something.
	MeshComp->OnComponentHit.AddDynamic(this, &ATPSProjectile::OnHit);
	RootComponent = MeshComp;

	ProjectileMoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMoveComp"));
	ProjectileMoveComp->bShouldBounce = true;
	ProjectileMoveComp->MaxSpeed = 2000;
	ProjectileMoveComp->InitialSpeed = 2000;

	ExplosionDamage = 100;
	ExplosionRadius = 300;
	ExplosionDelay = 1;
}

// Called when the game starts or when spawned
void ATPSProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Explode after delay if we did not hit target
	FTimerHandle TimerHandle_Explode;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Explode, this, &ATPSProjectile::Explode, ExplosionDelay, false);
}

void ATPSProjectile::Explode()
{
	// Apply radial damage to nearby actors
	TArray<AActor*> IgnoredActors;
	UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, UDamageType::StaticClass(), IgnoredActors, this, nullptr, false, ECC_Visibility);
	DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Yellow, false, 1.0f, 0, 2.0f);

	// Play FX
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	// Destroy Actor
	Destroy();
}

// Function that is called when the projectile hits something.
void ATPSProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor != this)
	{
		UTPSHealthComponent* OtherHealthComp = OtherActor->FindComponentByClass<UTPSHealthComponent>();
		if(OtherHealthComp)
		{
			if(OtherHealthComp->GetCurrentHealth() > 0)
			{
				Explode();
			}
		}		
	}
}

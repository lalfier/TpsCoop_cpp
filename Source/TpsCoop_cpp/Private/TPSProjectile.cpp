// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Components/TPSHealthComponent.h"


// Console variable
static int32 DrawProjectileDebugLinesValue = 0;
FAutoConsoleVariableRef DrawProjectileDebugDebugLines(TEXT("COOP.DebugProjectile"), DrawProjectileDebugLinesValue, TEXT("Draw Debug Lines for Projectile"), ECVF_Cheat);


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

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 200.0f;
	RadialForceComp->ImpulseStrength = 300.0f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;	// Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true;	// Ignore self

	ProjectileMoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMoveComp"));
	ProjectileMoveComp->bShouldBounce = true;
	ProjectileMoveComp->MaxSpeed = 2000;
	ProjectileMoveComp->InitialSpeed = 2000;

	ExplosionDamage = 100.0f;
	ExplosionRadius = 200.0f;
	ExplosionDelay = 1.0f;

	SetReplicates(true);
	SetReplicateMovement(true);
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
	if(bExploded)
	{
		return;
	}

	// Explode!
	bExploded = true;

	// Draw Sphere
	if(DrawProjectileDebugLinesValue > 0)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Yellow, false, 1.0f, 0, 2.0f);
	}	

	// Play FX
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	// Hide Mesh
	MeshComp->SetVisibility(false, true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if(HasAuthority())
	{
		// Blast away nearby physics actors
		RadialForceComp->FireImpulse();
		// Apply radial damage to nearby actors
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), false, ECC_Visibility);

		// Delete Actor with delay
		SetLifeSpan(2.0f);
	}	
}

void ATPSProjectile::OnRep_Exploded()
{
	// Draw Sphere
	if(DrawProjectileDebugLinesValue > 0)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Yellow, false, 1.0f, 0, 2.0f);
	}	

	// Hide Mesh
	MeshComp->SetVisibility(false, true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Play FX
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
}

// Function that is called when the projectile hits something.
void ATPSProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor && OtherActor != this)
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

// Apply rules for variable replications.
void ATPSProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This macro is default: replicate bExploded variable to all clients connected.
	DOREPLIFETIME(ATPSProjectile, bExploded);
}

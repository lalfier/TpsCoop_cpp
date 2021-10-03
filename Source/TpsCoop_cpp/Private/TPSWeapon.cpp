// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet//GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../TpsCoop_cpp.h"
#include "TimerManager.h"


// Console variable
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);


// Sets default values
ATPSWeapon::ATPSWeapon()
{
	// Setup weapon mesh
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// Setup weapon damage
	BaseDamage = 20.0f;
	DamageMultiplier = 4.0f;
	RateOfFire = 600;

	// Setup weapon effects
	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";
}

void ATPSWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

void ATPSWeapon::Fire()
{
	// Trace the world, from pawn eyes to cross-hair location
	AActor* WeaponOwner = GetOwner();
	if(WeaponOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(WeaponOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		// Target location for Particle system
		FVector TracerEndPoint = TraceEnd;

		FHitResult Hit;
		if(GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			// Blocking hit. Process damage
			AActor* HitActor = Hit.GetActor();

			// Select damage depending on hit surface type
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			float ActualDamage = BaseDamage;
			if(SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= DamageMultiplier;
			}
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, WeaponOwner->GetInstigatorController(), this, DamageType);

			// Select effect depending on hit surface type
			UParticleSystem* SelectedEffect = nullptr;
			switch (SurfaceType)
			{
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}
			// Play selected effect
			if(SelectedEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}

			TracerEndPoint = Hit.ImpactPoint;
		}

		if(DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void ATPSWeapon::StartFire()
{
	float FirstDelay = FMath::Max((LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds), 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ATPSWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ATPSWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ATPSWeapon::PlayFireEffects(FVector TraceEnd)
{
	// Draw Fire FX
	if(MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	// Draw Tracer FX
	if(TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerEffectComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if(TracerEffectComp)
		{
			TracerEffectComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	// Camera Shake
	AActor* WeaponOwner = Cast<APawn>(GetOwner());
	if(WeaponOwner)
	{
		APlayerController* PC = Cast<APlayerController>(WeaponOwner->GetInstigatorController());
		if(PC)
		{
			PC->ClientStartCameraShake(FireCamShake);
		}
	}
}

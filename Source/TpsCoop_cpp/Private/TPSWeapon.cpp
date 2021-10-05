// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSWeapon.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Kismet//GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../TpsCoop_cpp.h"
#include "TimerManager.h"


// Console variable
int32 ATPSWeapon::DrawWeaponDebugLinesValue = 0;
FAutoConsoleVariableRef ATPSWeapon::DrawWeaponDebugLines(TEXT("COOP.DebugWeapons"), DrawWeaponDebugLinesValue, TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);


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

	// When server spawns weapon, than spawn it on clients also
	bReplicates = true;
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ATPSWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

#pragma region FIRE
void ATPSWeapon::Fire()
{	
	if(!HasAuthority())
	{
		// If client calls this, push request to server
		ServerFire();
	}

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
		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		if(GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			// Blocking hit. Process damage
			AActor* HitActor = Hit.GetActor();

			// Select damage depending on hit surface type
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			float ActualDamage = BaseDamage;
			if(SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= DamageMultiplier;
			}
			if(HasAuthority())
			{
				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, WeaponOwner->GetInstigatorController(), this, DamageType);
			}			

			PlayImapctEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;
		}

		if(ATPSWeapon::DrawWeaponDebugLinesValue > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

		if(HasAuthority())
		{
			// As server save end point and surface type for other clients
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.HitSurfaceType = SurfaceType;
			HitScanTrace.bForceReplication = !HitScanTrace.bForceReplication;	// Force replication
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void ATPSWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ATPSWeapon::ServerFire_Validate()
{
	return true;
}

void ATPSWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX on other clients
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImapctEffects(HitScanTrace.HitSurfaceType, HitScanTrace.TraceTo);
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

void ATPSWeapon::PlayImapctEffects(EPhysicalSurface HitSurfaceType, FVector ImpactPoint)
{
	// Select effect depending on hit surface type
	UParticleSystem* SelectedEffect = nullptr;
	switch(HitSurfaceType)
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
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

#pragma endregion FIRE

// Apply rules for variable replications.
void ATPSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This macro is with condition: replicate HitScanTrace struct to all but not to client owning this weapon
	DOREPLIFETIME_CONDITION(ATPSWeapon, HitScanTrace, COND_SkipOwner);
}

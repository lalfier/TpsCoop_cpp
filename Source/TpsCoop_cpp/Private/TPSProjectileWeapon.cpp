// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSProjectileWeapon.h"
#include "DrawDebugHelpers.h"
#include "../TpsCoop_cpp.h"


void ATPSProjectileWeapon::Fire()
{
	if(!HasAuthority())
	{
		// If client calls this, push request to server and play FX
		ServerFire();
	}

	// Spawn weapon projectile
	AActor* WeaponOwner = GetOwner();
	if(WeaponOwner && ProjectileClass)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(WeaponOwner);
		QueryParams.AddIgnoredActor(this);

		// Target location with hit-scan
		FVector TracerEndPoint = TraceEnd;
		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		if(GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			TracerEndPoint = Hit.ImpactPoint;
		}

		if(ATPSWeapon::DrawWeaponDebugLinesValue > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);	

		if(HasAuthority())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Instigator = Cast<APawn>(WeaponOwner);
			SpawnParams.Owner = WeaponOwner;

			TracerEndPoint -= MuzzleLocation;
			TracerEndPoint.Normalize();
			GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, TracerEndPoint.Rotation(), SpawnParams);

			// As server save end point and surface type for other clients
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.HitSurfaceType = SurfaceType;
			HitScanTrace.bForceReplication = !HitScanTrace.bForceReplication;	// Force replication
		}

		PlayFireEffects(MuzzleLocation);

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

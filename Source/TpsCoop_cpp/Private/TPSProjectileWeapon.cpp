// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSProjectileWeapon.h"
#include "DrawDebugHelpers.h"
#include "../TpsCoop_cpp.h"


void ATPSProjectileWeapon::Fire()
{
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
		FHitResult Hit;
		if(GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			TracerEndPoint = Hit.ImpactPoint;
		}

		if(ATPSWeapon::DrawWeaponDebugLinesValue > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		TracerEndPoint -= MuzzleLocation;
		TracerEndPoint.Normalize();
		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, TracerEndPoint.Rotation(), SpawnParams);

		PlayFireEffects(MuzzleLocation);

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

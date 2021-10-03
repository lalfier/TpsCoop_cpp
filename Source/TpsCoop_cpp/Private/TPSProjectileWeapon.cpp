// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSProjectileWeapon.h"


void ATPSProjectileWeapon::Fire()
{
	// Spawn weapon projectile
	AActor* WeaponOwner = GetOwner();
	if(WeaponOwner && ProjectileClass)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);

		PlayFireEffects(MuzzleLocation);

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSWeapon.generated.h"

// Forward declaration
class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;


// Contains information of a single hit-scan weapon line-trace
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> HitSurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;

	// Helper bool to force replication even when the player shoots at the same spot.
	UPROPERTY()
	bool bForceReplication = false;
};


UCLASS()
class TPSCOOP_CPP_API ATPSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATPSWeapon();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShakeBase> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float DamageMultiplier;

	// RPM - Bullets per minute fired
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	// Bullet spread in degrees
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin=0.0f))
	float BulletSpread;

	// Derived from RateOfFire
	float TimeBetweenShots;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	// Weapon FX
	void PlayFireEffects(FVector TraceEnd);
	void PlayImapctEffects(EPhysicalSurface HitSurfaceType, FVector ImpactPoint);

	// Virtual so other weapons can override this
	virtual void Fire();

	// Does not run on client but push request to the server
	// Will get to the server
	// Validation for cheating
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:

	void StartFire();

	void StopFire();

public:
	static FAutoConsoleVariableRef DrawWeaponDebugLines;

protected:
	static int32 DrawWeaponDebugLinesValue;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSProjectile.generated.h"

// Forward declaration
class UStaticMeshComponent;
class UProjectileMovementComponent;
class URadialForceComponent;
class UParticleSystem;


UCLASS()
class TPSCOOP_CPP_API ATPSProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATPSProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMoveComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComp;

	// Particle to play when exploded
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionEffect;

	// Damage to other actors on explosion
	UPROPERTY(EditDefaultsOnly, Category = "ProjectileDamage")
	float ExplosionDamage;

	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	// Radius of damage explosion
	UPROPERTY(EditDefaultsOnly, Category = "ProjectileDamage")
	float ExplosionRadius;

	// Delay of explosion
	UPROPERTY(EditDefaultsOnly, Category = "ProjectileDamage")
	float ExplosionDelay;

	void Explode();

	UFUNCTION()
	void OnRep_Exploded();

	// Function that is called when the projectile hits something.
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

};

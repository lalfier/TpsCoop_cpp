// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSExplosiveBarrel.generated.h"


class UTPSHealthComponent;
class UStaticMeshComponent;
class URadialForceComponent;
class UParticleSystem;


UCLASS()
class TPSCOOP_CPP_API ATPSExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATPSExplosiveBarrel();

protected:

	UPROPERTY(VisibleAnywhere, Category = "BarrelHealth")
	UTPSHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComp;

	// Impulse applied to the barrel mesh when it explodes to boost it up a little
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionImpulse;

	// Particle to play when health reached zero
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionEffect;

	// The material to replace the original on the mesh once exploded (a blackened version)
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* ExplodedMaterial;

	bool bExploded;

	UFUNCTION()
	void OnHealthChanged(UTPSHealthComponent* InHealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

};
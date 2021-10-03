// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPSWeapon.h"
#include "TPSProjectileWeapon.generated.h"


UCLASS()
class TPSCOOP_CPP_API ATPSProjectileWeapon : public ATPSWeapon
{
	GENERATED_BODY()
	

protected:

	// Projectile class to spawn
	UPROPERTY(EditDefaultsOnly, Category = "ProjectileWeapon")
	TSubclassOf<AActor> ProjectileClass;

	// Override base weapon fire function
	virtual void Fire() override;

};

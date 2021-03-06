// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSCharacter.generated.h"

// Forward declaration
class UCameraComponent;
class USpringArmComponent;
class ATPSWeapon;
class UTPSHealthComponent;


UCLASS()
class TPSCOOP_CPP_API ATPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSCharacter();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerHealth")
	UTPSHealthComponent* HealthComp;

	// Pawn died previously
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

	// Pawn jumped previously
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bJumped;

	bool bWasCrouchKeyPressed;

	// Replicate for clients
	UPROPERTY(Replicated)
	ATPSWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerWeapon")
	TSubclassOf<ATPSWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "PlayerWeapon")
	FName WeaponAttachSocketName;

	bool bWantsToZoom;
	
	float DeafultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerWeapon")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerWeapon", meta = (ClampMin = 0.1, ClampMax = 100.0))
	float ZoomInterpSpeed;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called on input event
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrouch();
	void BeginJump();
	void BeginZoom();
	void EndZoom();

	// Event called
	UFUNCTION()
	void OnHealthChanged(UTPSHealthComponent* InHealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Override so we can use it on third person character
	virtual FVector GetPawnViewLocation() const override;

	// Change Weapon
	UFUNCTION(BlueprintCallable, Category = "Player")
	void ChangeWeapon(TSubclassOf<ATPSWeapon> WeaponClass);

	// Called on input event
	UFUNCTION(BlueprintCallable, Category = "Player")
	void BeginFire();
	UFUNCTION(BlueprintCallable, Category = "Player")
	void EndFire();

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSCharacter.generated.h"

// Forward declaration
class UCameraComponent;
class USpringArmComponent;
class ATPSWeapon;


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

	bool bWasCrouchKeyPressed;

	ATPSWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ATPSWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName WeaponAttachSocketName;

	bool bWantsToZoom;
	
	float DeafultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.1, ClampMax = 100.0))
	float ZoomInterpSpeed;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called on input event
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrouch();
	void BeginJump();
	void BeginFire();
	void EndFire();
	void BeginZoom();
	void EndZoom();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Override so we can use it on third person character
	virtual FVector GetPawnViewLocation() const override;

};

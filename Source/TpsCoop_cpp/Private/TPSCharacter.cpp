// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TPSHealthComponent.h"
#include "../TpsCoop_cpp.h"
#include "TPSWeapon.h"


// Sets default values
ATPSCharacter::ATPSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup camera
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// Enable crouch (UE4 function)
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	bWasCrouchKeyPressed = false;

	// Setup jump
	bJumped = false;

	// Setup collision
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	// Setup health
	HealthComp = CreateDefaultSubobject<UTPSHealthComponent>(TEXT("HealthComp"));

	// Setup weapon/zoom
	WeaponAttachSocketName = "WeaponSocket";
	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;
}

// Called when the game starts or when spawned
void ATPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Save starting camera FOV
	DeafultFOV = CameraComp->FieldOfView;

	// Subscribe to an event
	HealthComp->OnHealthChanged.AddDynamic(this, &ATPSCharacter::OnHealthChanged);

	if(HasAuthority())
	{
		// Spawn a default weapon as server
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<ATPSWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if(CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}
}

void ATPSCharacter::OnHealthChanged(UTPSHealthComponent* InHealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if(CurrentHealth <= 0.0f && !bDied)
	{
		// Die!
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetachFromControllerPendingDestroy();
		SetLifeSpan(5.0f);
		EndFire();
		CurrentWeapon->SetLifeSpan(5.0f);
	}
}

// Called on input event
#pragma region MOVEMENT
void ATPSCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ATPSCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ATPSCharacter::BeginCrouch()
{
	bWasCrouchKeyPressed = true;
	if(GetMovementComponent()->IsMovingOnGround())
	{
		Crouch();
	}	
}

void ATPSCharacter::EndCrouch()
{
	bWasCrouchKeyPressed = false;
	UnCrouch();
}

void ATPSCharacter::BeginJump()
{
	Jump();
}
#pragma endregion MOVEMENT

#pragma region WEAPON
void ATPSCharacter::BeginFire()
{
	if(CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ATPSCharacter::EndFire()
{
	if(CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ATPSCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ATPSCharacter::EndZoom()
{
	bWantsToZoom = false;
}
#pragma endregion WEAPON

// Called every frame
void ATPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Got to crouch while landing if key was pressed in air
	if(!GetCharacterMovement()->IsCrouching())
	{
		if(bWasCrouchKeyPressed)
		{
			BeginCrouch();
		}
	}

	// Check if player ADS
	float TargetFOV = bWantsToZoom ? ZoomedFOV : DeafultFOV;
	// Smooth transition with interpolation
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);

	if(bWasJumping && !bJumped)
	{
		bJumped = true;
	}
	else if(GetCharacterMovement()->IsMovingOnGround() && bJumped)
	{
		bJumped = false;
	}

}

// Called to bind functionality to input
void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Keyboard move
	PlayerInputComponent->BindAxis("MoveForward", this, &ATPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATPSCharacter::MoveRight);

	// Mouse look/turn
	PlayerInputComponent->BindAxis("LookUp", this, &ATPSCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ATPSCharacter::AddControllerYawInput);

	// Actions
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATPSCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ATPSCharacter::EndCrouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATPSCharacter::BeginJump);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATPSCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATPSCharacter::EndFire);
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ATPSCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ATPSCharacter::EndZoom);
}

// Override so we can use it on third person character
FVector ATPSCharacter::GetPawnViewLocation() const
{
	if(CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

// Apply rules for variable replications.
void ATPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This macro is default: replicate CurrentWeapon, bDied, bJumped variable to all clients connected.
	DOREPLIFETIME(ATPSCharacter, CurrentWeapon);
	DOREPLIFETIME(ATPSCharacter, bDied);
	DOREPLIFETIME(ATPSCharacter, bJumped);
}

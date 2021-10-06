// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TPSTrackerBot.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "TPSCharacter.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Components/TPSHealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ATPSTrackerBot::ATPSTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<UTPSHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ATPSTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 300.0f;
	RadialForceComp->ImpulseStrength = 450.0f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;	// Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true;	// Ignore self

	bUseVelocityChange = true;
	MovementForce = 500;
	RequiredDistanceToTarget = 100;
	ExplosionDamage = 100;
	ExplosionRadius = 300;
	SelfDamageInterval = 0.25f;
	MaxPowerLevel = 4;
}

// Called when the game starts or when spawned
void ATPSTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	if(HasAuthority())
	{
		// Find initial move-to
		NextPathPoint = GetNextPathPoint();

		// Every second we update our power-level based on nearby bots
		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ATPSTrackerBot::OnCheckNearbyBots, 1.0f, true);
	}
}

void ATPSTrackerBot::HandleTakeDamage(UTPSHealthComponent* InHealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// Pulse the material on hit
	if(MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}	
	if(MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(CurrentHealth), *GetName());

	// Explode on hitpoints == 0
	if(CurrentHealth <= 0.0f)
	{
		SelfDestruct();
	}
}

FVector ATPSTrackerBot::GetNextPathPoint()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	UNavigationPath* NavPath =  UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	// Return next point in the path
	if(NavPath->PathPoints.Num() > 1)
	{
		return NavPath->PathPoints[1];
	}

	// Failed to find path
	return GetActorLocation();
}

void ATPSTrackerBot::SelfDestruct()
{
	if(bExploded)
	{
		return;
	}

	// Explode!
	bExploded = true;

	// Draw Sphere
	DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 1.0f, 0, 2.0f);

	// Play FX
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	// Play sound
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	// Hide Mesh
	MeshComp->SetVisibility(false, true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if(HasAuthority())
	{
		// Blast away nearby physics actors
		RadialForceComp->FireImpulse();
		// Apply radial damage
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		// Increase damage based on the power level
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), false, ECC_Visibility);

		// Delete Actor with delay
		SetLifeSpan(2.0f);
	}
}

void ATPSTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ATPSTrackerBot::OnRep_Exploded()
{
	// Draw Sphere
	DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 1.0f, 0, 2.0f);

	// Play FX
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	// Play sound
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	// Hide Mesh
	MeshComp->SetVisibility(false, true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATPSTrackerBot::OnCheckNearbyBots()
{
	// Distance to check for nearby bots
	static constexpr float Radius = 600;

	// Create temporary collision shape for overlaps
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);

	// Only find Pawns (like players and AI bots)
	FCollisionObjectQueryParams QueryParams;
	// Our tracker bots mesh component is set to Physics Body in Blueprint (default profile of physics simulated actors)
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);
	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);

	int32 NrOfBots = 0;
	// Loop over the result using a "range based for loop"
	for(FOverlapResult Result : Overlaps)
	{
		// Check if we overlapped with another tracker bot (ignoring players and other bot types)
		ATPSTrackerBot* Bot = Cast<ATPSTrackerBot>(Result.GetActor());
		// Ignore this tracker bot instance
		if(Bot && Bot != this)
		{
			NrOfBots++;
		}
	}

	// Clamp between 0 and 4
	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);

	// Update the material color
	if(MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if(MatInst)
	{
		// Convert to a float between 0 and 1 just like an 'Alpha' value of a texture. Now the material can be set up without having to know the max power level 
		// which can be tweaked many times by game-play decisions (would mean we need to keep 2 places up to date)
		float Alpha = PowerLevel / (float)MaxPowerLevel;
		// Note: (float)MaxPowerLevel converts the int32 to a float, 
		//	otherwise the following happens when dealing when dividing integers: 1 / 4 = 0 ('PowerLevel' int / 'MaxPowerLevel' int = 0 int)
		//	this is a common programming problem and can be fixed by 'casting' the int (MaxPowerLevel) to a float before dividing.

		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}

	// Draw on bot location
	DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
}

void ATPSTrackerBot::OnRep_PowerLevel()
{
	// Update material "PowerLevelAlpha" variable
	if(MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if(MatInst)
	{
		float Alpha = PowerLevel / (float)MaxPowerLevel;
		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}
}

// Called every frame
void ATPSTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(HasAuthority() && !bExploded)
	{
		float DistanceToTaret = (GetActorLocation() - NextPathPoint).Size();
		if(DistanceToTaret <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();

			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!", nullptr, FColor::White, 1.0f, false, 1.0f);
		}
		else
		{
			// Keep moving towards next target
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
	}
}

void ATPSTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if(!bStartedSelfDestruction && !bExploded)
	{
		ATPSCharacter* PlayerPawn = Cast<ATPSCharacter>(OtherActor);
		if(PlayerPawn)
		{
			// We overlapped with a player, start self destruction sequence
			if(HasAuthority())
			{
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ATPSTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}
			
			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}

// Apply rules for variable replications.
void ATPSTrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This macro is default: replicate bExploded, PowerLevel variable to all clients connected.
	DOREPLIFETIME(ATPSTrackerBot, bExploded);
	DOREPLIFETIME(ATPSTrackerBot, PowerLevel);
}

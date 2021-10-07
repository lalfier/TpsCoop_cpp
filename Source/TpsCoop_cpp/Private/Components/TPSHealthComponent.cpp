// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TPSHealthComponent.h"
#include "TPSGameMode.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UTPSHealthComponent::UTPSHealthComponent()
{
	bIsDead = false;
	MaxHealth = 100.0f;

	// Replicate this component
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UTPSHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if(GetOwnerRole() == ROLE_Authority)
	{
		// Subscribe to OnTakeDamage function event from owner, but only if this is server
		AActor* HealthOwner = GetOwner();
		if(HealthOwner)
		{
			HealthOwner->OnTakeAnyDamage.AddDynamic(this, &UTPSHealthComponent::HandleTakeAnyDamage);
		}
	}

	CurrentHealth = MaxHealth;
}

void UTPSHealthComponent::OnRep_CurrentHealth(float OldHealth)
{
	float DeltaHealth = CurrentHealth - OldHealth;
	OnHealthChanged.Broadcast(this, CurrentHealth, DeltaHealth, nullptr, nullptr, nullptr);
}

void UTPSHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if(Damage <= 0.0f || bIsDead)
	{
		return;
	}

	// Update health clamped
	CurrentHealth = FMath::Clamp((CurrentHealth - Damage), 0.0f, MaxHealth);
	// Set is dead
	bIsDead = CurrentHealth <= 0.0f;

	UE_LOG(LogTemp, Log, TEXT("%s Health Changed: %s"), *DamagedActor->GetName(), *FString::SanitizeFloat(CurrentHealth));

	OnHealthChanged.Broadcast(this, CurrentHealth, -Damage, DamageType, InstigatedBy, DamageCauser);

	if(bIsDead)
	{
		ATPSGameMode* GM = Cast<ATPSGameMode>(GetWorld()->GetAuthGameMode());
		if(GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
}

void UTPSHealthComponent::ResetCurrentHeath()
{
	CurrentHealth = MaxHealth;
}

void UTPSHealthComponent::HealPlayer(float HealAmount)
{
	if(HealAmount <= 0.0f || CurrentHealth <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(CurrentHealth), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, CurrentHealth, HealAmount, nullptr, nullptr, nullptr);
}

// Apply rules for variable replications.
void UTPSHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This macro is default: replicate CurrentHealth variable to all clients connected.
	DOREPLIFETIME(UTPSHealthComponent, CurrentHealth);
}

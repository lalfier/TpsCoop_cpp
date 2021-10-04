// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TPSHealthComponent.h"


// Sets default values for this component's properties
UTPSHealthComponent::UTPSHealthComponent()
{
	MaxHealth = 100.0f;
}


// Called when the game starts
void UTPSHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Subscribe to OnTakeDamage function event from owner
	AActor* HealthOwner = GetOwner();
	if(HealthOwner)
	{
		HealthOwner->OnTakeAnyDamage.AddDynamic(this, &UTPSHealthComponent::HandleTakeAnyDamage);
	}

	CurrentHealth = MaxHealth;
}

void UTPSHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if(Damage <= 0.0f)
	{
		return;
	}

	// Update health clamped
	CurrentHealth = FMath::Clamp((CurrentHealth - Damage), 0.0f, MaxHealth);

	UE_LOG(LogTemp, Log, TEXT("%s Health Changed: %s"), *DamagedActor->GetName(), *FString::SanitizeFloat(CurrentHealth));

	OnHealthChanged.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);
}

void UTPSHealthComponent::ResetCurrentHeath()
{
	CurrentHealth = MaxHealth;
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "MovementSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UMovementSystem::UMovementSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMovementSystem::BeginPlay()
{
	Super::BeginPlay();

	// 1. Get owner
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 2. Cast to Character
	OwningCharacter = Cast<ACharacter>(Owner);
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Owner is not a Character"), *GetName());
		return;
	}

	// 3. Cache Movement Component
	MovementComponent = OwningCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Character has no MovementComponent"), *GetName());
	}
	
	// ...
	
}


// Called every frame
void UMovementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


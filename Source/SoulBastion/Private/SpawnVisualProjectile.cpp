// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVisualProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

USpawnVisualProjectile* USpawnVisualProjectile::SpawnVisualProjectile(UObject* WorldContext, FVector StartLocation, FVector Direction, float Speed, float Lifetime, UNiagaraSystem* NiagaraSystem, AActor* IgnoreActor)
{
	if (!WorldContext || !NiagaraSystem) return nullptr;
	
	USpawnVisualProjectile* Task = NewObject<USpawnVisualProjectile>();
	
	Task->World = WorldContext->GetWorld();
	Task->IgnoredActor = IgnoreActor;
	
	Task->CurrentLocation = StartLocation;
	Task->Velocity = Direction.GetSafeNormal() * Speed;
	Task->MaxLifetime = Lifetime;
	
	Task->NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(Task->World.Get(), NiagaraSystem, StartLocation);
	
	Task->bIsActive = true;
	return Task;
}
void USpawnVisualProjectile::Tick(float DeltaTime)
{
	if (!World.IsValid() || !NiagaraComp)
	{
		EndTask();
		return;
	}

	ElapsedTime += DeltaTime;
	if (ElapsedTime >= MaxLifetime)
	{
		EndTask();
		return;
	}

	FVector PreviousLocation = CurrentLocation;
	CurrentLocation += Velocity * DeltaTime;

	// Trace
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(IgnoredActor.Get());

	bool bHit = World->LineTraceSingleByChannel(
		Hit,
		PreviousLocation,
		CurrentLocation,
		ECC_Visibility,
		Params
	);

	// Move Niagara
	NiagaraComp->SetWorldLocation(CurrentLocation);

	if (bHit)
	{
		OnHit.Broadcast(Hit);
		EndTask();
	}
}
void USpawnVisualProjectile::EndTask()
{
	bIsActive = false;

	if (NiagaraComp)
	{
		NiagaraComp->Deactivate();
	}

	OnFinished.Broadcast();
	SetReadyToDestroy();
}
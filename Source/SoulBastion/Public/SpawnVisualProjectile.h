// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Tickable.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "SpawnVisualProjectile.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVisualProjectileHit, const FHitResult&, Hit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVisualProjectileFinished);
UCLASS()
class SOULBASTION_API USpawnVisualProjectile : public UBlueprintAsyncActionBase, public FTickableGameObject
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintAssignable)
	FVisualProjectileHit OnHit;
	
	UPROPERTY(BlueprintAssignable)
	FVisualProjectileFinished OnFinished;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static USpawnVisualProjectile* SpawnVisualProjectile(UObject* WorldContext, FVector StartLocation, FVector Direction, float Speed, float Lifetime, UNiagaraSystem* NiagaraSystem, AActor* IgnoreActor);

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bIsActive; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(USpawnVisualProjectile, STATGROUP_Tickables); }

protected:
	void EndTask();

private:
	bool bIsActive = false;

	FVector CurrentLocation;
	FVector Velocity;
	float MaxLifetime;
	float ElapsedTime = 0.f;

	UPROPERTY()
	UNiagaraComponent* NiagaraComp = nullptr;
	
	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<AActor> IgnoredActor;
	
};

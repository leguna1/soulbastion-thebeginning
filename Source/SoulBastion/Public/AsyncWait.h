// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncWait.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitEvent);
UCLASS()
class SOULBASTION_API UAsyncWait : public UBlueprintAsyncActionBase, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FWaitEvent OnFinished;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UAsyncWait* Wait(UObject* WorldContextObject, float Time);
	
	virtual void Activate() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	
private:
	UPROPERTY()
	UObject* WorldContextObject;
	
	float TotalTime = 0.f;
	float ElapsedTime = 0.f;
	
	bool bIsActive = false;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncWait.h"
#include "Engine/World.h"

UAsyncWait* UAsyncWait::Wait(UObject* WorldContextObject, const float Time)
{
	UAsyncWait* Node = NewObject<UAsyncWait>();
	Node->WorldContextObject = WorldContextObject;
	Node->TotalTime = FMath::Max(Time, 0.f);
	return Node;
}
void UAsyncWait::Activate()
{
	if (!WorldContextObject)
	{
		SetReadyToDestroy();
		return;
	}
	
	ElapsedTime = 0.f;
	bIsActive = true;
}
void UAsyncWait::Tick(float DeltaTime)
{
	if (!bIsActive) return;
	
	ElapsedTime += DeltaTime;
	
	if (ElapsedTime >= TotalTime)
	{
		bIsActive = false;
		OnFinished.Broadcast();
		
		SetReadyToDestroy();
	}
}

bool UAsyncWait::IsTickable() const
{
	return bIsActive && IsValid(this);
}
TStatId UAsyncWait::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAsyncWait, STATGROUP_Tickables);
}
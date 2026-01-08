// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillBase.h"

#include "StatSystem.h"

void USkillBase::Tick(float DeltaTime)
{
	// Only process tick if at least one system requested it
	if (TickRequests <= 0)
		return;

	// Recharge logic
	Recharge(DeltaTime);
	
}
TStatId USkillBase::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USkillBase, STATGROUP_Tickables);
}
bool USkillBase::IsTickable() const
{
	return TickRequests > 0;
}
void USkillBase::RequestTick()
{
	++TickRequests;
	
}

void USkillBase::ReleaseTick()
{
	if (TickRequests > 0)
	{
		--TickRequests;
		
	}
}

bool USkillBase::CanActivate_Implementation(FGameplayTag Tag)
{
	float CurrentEnergy = GetOwningAbility() -> GetOwnerStats() -> GetStatValue(FGameplayTag::RequestGameplayTag("Stat.Energy"), EStatValueType::Value);
	
	if (Tag == SkillTag && SkillData.MaxCharge > 0 && SkillData.CurrentCharge > 0 && CurrentEnergy >= SkillData.EnergyCost)
	{
		
		return true;
	}
	return false;
	
}

void USkillBase::BeginRecharge(int32 ChargeToConsume)
{
	if (SkillData.CurrentCharge <= 0)
		return;

	int32 ActualConsume = FMath::Clamp(ChargeToConsume, 0, SkillData.CurrentCharge);
	SkillData.CurrentCharge -= ActualConsume;

	// Start tick
	RequestTick();

	// Multi-charge: only start timer if it is not already running
	if (SkillData.MaxCharge > 1)
	{
		if (CurrentRechargeTime <= 0.f)
			CurrentRechargeTime = 0.f;
		// else leave timer running
	}
	else // Single-charge
	{
		CurrentRechargeTime = SkillData.RechargeTime;
	}
}

void USkillBase::Recharge(float DeltaTime)
{
	if (SkillData.CurrentCharge >= SkillData.MaxCharge)
	{
		ReleaseTick();
		return;
	}

	if (SkillData.MaxCharge > 1)
	{
		// MULTI-CHARGE: increment timer, add charges for full recharge periods
		CurrentRechargeTime += DeltaTime;

		while (CurrentRechargeTime >= SkillData.RechargeTime && SkillData.CurrentCharge < SkillData.MaxCharge)
		{
			++SkillData.CurrentCharge;
			CurrentRechargeTime -= SkillData.RechargeTime;
			
		}

		// Clamp the timer for safety
		CurrentRechargeTime = FMath::Clamp(CurrentRechargeTime, 0.f, SkillData.RechargeTime);
	}
	else
	{
		// SINGLE-CHARGE: cooldown timer counts down
		CurrentRechargeTime -= DeltaTime;

		if (CurrentRechargeTime <= 0.f)
		{
			SkillData.CurrentCharge = 1;
			CurrentRechargeTime = 0.f;
			ReleaseTick();
			
		}

		CurrentRechargeTime = FMath::Clamp(CurrentRechargeTime, 0.f, SkillData.RechargeTime);
	}

	// Broadcast current recharge time
	if (AbilitySystemRef)
		AbilitySystemRef->OnChargeTimeUpdated.Broadcast(SkillTag, SkillData.CurrentCharge, SkillData.RechargeTime, CurrentRechargeTime);
}

UUtilityBox* USkillBase::GetUtilityBox() const
{
	return OwningCharacterRef->FindComponentByClass<UUtilityBox>();
}

void USkillBase::OnOwnerHitResponse_Implementation(FHitInfo Payload)
{
	//Handled in blueprint by children
}

void USkillBase::OnOwnerStatChanged_Implementation(FStatChangedEvent Payload)
{
	//Handled in blueprint by children
}
void USkillBase::OnOwnerDeath_Implementation(FOnDeathEvent Payload)
{
	//Handled in blueprint by children.
}

void USkillBase::OnActivation_Implementation(FGameplayTag Tag, EActivationInput Input, FVector2D InputAction, float ElapsedTime)
{
	//Handled in blueprint by children.
}
void USkillBase::OnSkillStateChanged_Implementation(FGameplayTag EventSkillTag, ESkillState NewState, float NewDuration)
{
	//Handled in blueprint by children.
}
void USkillBase::OnAnimMontageEvent_Implementation(FAnimMontageData EventData)
{
	//AnimMontage events.
}
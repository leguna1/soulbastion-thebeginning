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

	// Forward to blueprint (for other tick-driven effects like particle updates etc.)
	OnSkillTick(DeltaTime);
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
	DEBUG_LOG("%s: Tick requested. Count = %d", *GetName(), TickRequests);
}

void USkillBase::ReleaseTick()
{
	if (TickRequests > 0)
	{
		--TickRequests;
		DEBUG_LOG("%s: Tick released. Count = %d", *GetName(), TickRequests);
	}
}
bool USkillBase::CanActivate_Implementation(FGameplayTag Tag)
{
	
	bool IsAlive = GetOwningAbility() -> GetOwnerStats() -> IsAlive();
	float CurrentEnergy = GetOwningAbility() -> GetOwnerStats() -> GetStatCurrentValue(FGameplayTag::RequestGameplayTag("Stat.Energy"));
	
	if (SkillData.MaxCharge > 0 && SkillData.CurrentCharge > 0 && IsAlive && CurrentEnergy >= SkillData.EnergyCost)
	{
		DEBUG_LOG("CanActivate: All check points passed, can activate skill %s", *SkillTag.ToString());
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

	// Multi-charge: only start timer if it wasn’t already running
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
			DEBUG_LOG("%s: Recharged 1 charge. CurrentCharge = %d/%d", *GetName(), SkillData.CurrentCharge, SkillData.MaxCharge);
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
			DEBUG_LOG("%s: Single-charge skill fully recharged.", *GetName());
		}

		CurrentRechargeTime = FMath::Clamp(CurrentRechargeTime, 0.f, SkillData.RechargeTime);
	}

	// Broadcast current recharge time
	if (AbilitySystemRef)
		AbilitySystemRef->OnChargeTimeUpdated.Broadcast(SkillTag, SkillData.CurrentCharge, SkillData.RechargeTime, CurrentRechargeTime);
}

void USkillBase::OnSkillConstruct_Implementation()
{
	//Skill begin play.
}

void USkillBase::OnActivation_Implementation(FGameplayTag Tag, EActivationInput Input, float ElapsedTime)
{
	//Handled in blueprint by children.
}
void USkillBase::OnSkillStateChanged_Implementation(FGameplayTag EventSkillTag, ESkillState NewState)
{
	//Handled in blueprint by children.
}
void USkillBase::OnAnimMontageEvent_Implementation(FAnimMontageData EventData)
{
	//AnimMontage events.
}
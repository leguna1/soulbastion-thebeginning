#include "StatSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"


UStatSystem::UStatSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicated(true);
}

void UStatSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind to stat changed event
    OnStatChanged.AddDynamic(this, &UStatSystem::HandleStatChanged);

    // Start passive regen
    GetWorld()->GetTimerManager().SetTimer(
        RecoveryTimerHandle,
        this,
        &UStatSystem::Recovery,
        1.0f,    // every 1 second
        true
    );
}

void UStatSystem::HandleStatChanged(FStatChangedEvent StatEvent)
{
    // Only react if value decreased
    if (StatEvent.ChangeValue < 0.f)
    {
        EnterCombat();
    }
}

void UStatSystem::GetStatValues(FGameplayTag Tag, float& OutValue, float& OutMaxValue) const
{
    for (const FStatData& Stat : Stats)
    {
        if (Stat.StatTag == Tag)
        {
            OutValue = Stat.Value;
            OutMaxValue = Stat.MaxValue;
            return;
        }
    }

    OutValue = 0.f;
    OutMaxValue = 0.f;
}

// -------------------------
// Modify only Value
// -------------------------
void UStatSystem::ModifyStatValue(FGameplayTag Tag, float DeltaValue, AActor* SourceModifier)
{
    for (FStatData& Stat : Stats)
    {
        if (Stat.StatTag == Tag)
        {
            if (FMath::IsNearlyZero(DeltaValue)) return;

            Stat.OldValue = Stat.Value;
            Stat.Value = FMath::Clamp(Stat.Value + DeltaValue, 0.f, Stat.MaxValue);

            // Prepare stat changed event
            FStatChangedEvent Event;
            Event.StatTag = Tag;
            Event.ChangeValue = Stat.Value - Stat.OldValue;
            Event.SourceModifier = SourceModifier;

            OnStatChanged.Broadcast(Event);

            // Check for death if health reaches 0
            if (Tag == FGameplayTag::RequestGameplayTag("Stat.Health") && Stat.Value <= 0.f && bIsAlive)
            {
                bIsAlive = false;

                FOnDeathEvent DeathEvent;
                DeathEvent.Killer = SourceModifier;
                DeathEvent.Damage = Event.ChangeValue;

                OnDeath.Broadcast(DeathEvent);
            }

            if (Tag == FGameplayTag::RequestGameplayTag("Stat.Health") && Stat.Value > 0.f && !bIsAlive)
            {
                bIsAlive = true;
            }
            return;
        }
    }
}

// -------------------------
// Modify only MaxValue
// -------------------------
void UStatSystem::ModifyStatMaxValue(FGameplayTag Tag, float DeltaMaxValue, AActor* SourceModifier)
{
    for (FStatData& Stat : Stats)
    {
        if (Stat.StatTag == Tag)
        {
            if (FMath::IsNearlyZero(DeltaMaxValue)) return;

            Stat.OldMaxValue = Stat.MaxValue;
            Stat.MaxValue += DeltaMaxValue;

            // Clamp current Value to new MaxValue
            Stat.Value = FMath::Clamp(Stat.Value, 0.f, Stat.MaxValue);

            // Prepare event
            FStatChangedEvent Event;
            Event.StatTag = Tag;
            Event.ChangeValue = DeltaMaxValue;
            Event.SourceModifier = SourceModifier;

            OnStatChanged.Broadcast(Event);
            
            return;
        }
    }
}
void UStatSystem::EnterCombat()
{
    if (!bInCombat)
    {
        bInCombat = true;
    }

    // Reset combat timeout every time damage/energy loss happens
    GetWorld()->GetTimerManager().ClearTimer(CombatTimeoutHandle);

    GetWorld()->GetTimerManager().SetTimer(
        CombatTimeoutHandle,
        this,
        &UStatSystem::LeaveCombat,
        6.0f,
        false
    );
}

void UStatSystem::LeaveCombat()
{
    bInCombat = false;

    // Cannot stay resting in combat but once out of combat resting allowed
    // (Resting mode is manually controlled)
}

float UStatSystem::ComputeRecoveryMultiplier() const
{
    float RecoveryValue, RecoveryMax;
    GetStatValues(FGameplayTag::RequestGameplayTag("Stat.Recovery"), RecoveryValue, RecoveryMax);

    // Diminishing returns curve
    float Diminished = RecoveryValue / (RecoveryValue + 100);

    float ModeMultiplier = 0.f;

    if (bInCombat)
        ModeMultiplier = 1.0f;
    else if (bIsResting)
        ModeMultiplier = 10.0f;
    else
        ModeMultiplier = 3.0f;

    return Diminished * ModeMultiplier;
}


void UStatSystem::Recovery()
{
    if (!IsAlive())
        return;

    // Regen amounts per second
    constexpr float BaseHealthPer5s = 3.f;
    constexpr float HealthPerSecond = BaseHealthPer5s / 5.f; // 0.6

    float Health, MaxHealth;
    float Energy, MaxEnergy;

    GetStatValues(FGameplayTag::RequestGameplayTag("Stat.Health"), Health, MaxHealth);
    GetStatValues(FGameplayTag::RequestGameplayTag("Stat.Energy"), Energy, MaxEnergy);

    float RecoveryMult = ComputeRecoveryMultiplier();

    // Health Regen
    if (Health < MaxHealth)
    {
        float RegenAmount = HealthPerSecond * (1.f + RecoveryMult);
        ModifyStatValue(FGameplayTag::RequestGameplayTag("Stat.Health"), RegenAmount, nullptr);
    }

    // Energy Regen
    if (Energy < MaxEnergy)
    {
        constexpr float EnergyPerSecond = 1.f;
        float RegenAmount = EnergyPerSecond * (1.f + RecoveryMult);
        ModifyStatValue(FGameplayTag::RequestGameplayTag("Stat.Energy"), RegenAmount, nullptr);
    }
  
}

void UStatSystem::SetResting(bool bRest)
{
    // Cannot rest in combat
    if (bInCombat)
    {
        bIsResting = false;
        return;
    }
    bIsResting = bRest;
}
bool UStatSystem::TakeDamage(AActor* DamageDealer, FVector ImpactPoint, float DamageAmount)
{
    if (IsDamageImmune || DamageAmount <= 0.f) { return false; }

    const FGameplayTag HealthTag = FGameplayTag::RequestGameplayTag("Stat.Health");
    const FGameplayTag ArmorTag  = FGameplayTag::RequestGameplayTag("Stat.Armor");

    if (GetStatCurrentValue(HealthTag) <= 0.f) { return false; }

    float DamageAfterArmor = FMath::Max(0.f, DamageAmount - GetStatCurrentValue(ArmorTag));

    // Clean. Simple. Perfect.
    ModifyStatValue(HealthTag, -DamageAfterArmor, DamageDealer);

    // Optional: broadcast hit info for VFX
    FStatChangedEvent Event;
    Event.StatTag = HealthTag;
    Event.ChangeValue = -DamageAfterArmor;
    Event.SourceModifier = DamageDealer;
    OnStatChanged.Broadcast(Event);
    
    return true;
    
}
float UStatSystem::GetStatCurrentValue(FGameplayTag Stat) const
{
    for (const FStatData& Data : Stats)
    {
        if (Data.StatTag == Stat)
        {
            float Value, MaxValue;
            GetStatValues(Stat, Value, MaxValue);
            return Value;
        }
    }
    return 0.f;
}



#include "StatSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DSP/MidiNoteQuantizer.h"
#include "Kismet/GameplayStatics.h"


UStatSystem::UStatSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicated(true);
}

void UStatSystem::BeginPlay()
{
    Super::BeginPlay();
    
    AbilitySystem = GetOwner()->FindComponentByClass<UAbilitySystem>();
    UtilityBox = GetOwner()->FindComponentByClass<UUtilityBox>();
    
    // Initialize Old Values for all stats
    for (FStatData& Stat : Stats)
    {
        Stat.BaseValue += Stat.BonusValue;
        
        Stat.BaseValue = FMath::Clamp(Stat.BaseValue, 0.f, Stat.BaseMaxValue); 
        
        Stat.BaseMaxValue += Stat.BonusMaxValue;
    }
    
}

float UStatSystem::GetStatValue(const FGameplayTag Tag, const EStatValueType Type) const
{
    for (const FStatData& Stat : Stats)
    {
        if (Stat.StatTag == Tag)
        {
            switch (Type)
            {
            
            case EStatValueType::Value:
                return Stat.BaseValue + Stat.BonusValue;
            
            case EStatValueType::BaseValue:
                return Stat.BaseValue;
                
            case EStatValueType::BonusValue:
                return Stat.BonusValue; 
                
            case EStatValueType::MaxValue:
                return Stat.BaseMaxValue + Stat.BonusMaxValue;
            
            case EStatValueType::BaseMaxValue:
                return Stat.BaseMaxValue;
                
            case EStatValueType::BonusMaxValue:
                return Stat.BonusMaxValue;
                
            default: 
                return 0.f;
            }
        }
    }
    return 0.f;
}

FText UStatSystem::GetStatDetails(const FGameplayTag Tag) const
{
    for (const FStatData& Stat : Stats)
    {
        if (Stat.StatTag == Tag)
        {
            return Stat.Description;
        }
    }
    return FText::GetEmpty();
}

void UStatSystem::GainEssence(float RawEssenceAmount)
{
    if (RawEssenceAmount <= 0.f)
    {
        return;
    }
    const float EssenceScalar = GetEssenceGainScalar();
    
    const float ScaledEssence = RawEssenceAmount * EssenceScalar;
    
    SoulForceData.GainedSoulEssences += ScaledEssence;
    
    ScaleStats(ScaledEssence);
    
    OnSoulForceChanged.Broadcast(SoulForceData.GainedSoulEssences);
}
float UStatSystem::GetEssenceGainScalar() const
{
    if (SoulForceData.EssenceGainScalarCurve)
    {
        return SoulForceData.EssenceGainScalarCurve
            ->GetFloatValue(SoulForceData.SoulForceLevel);
    }

    return 1.f; 
}
void UStatSystem::SoulForceDeathPenalty()
{
    if (SoulForceData.GainedSoulEssences <= 0) return;
    
    const float LossPercent = GetEssenceLossPercent();
    
    if (LossPercent <= 0.f) return;
    
    const float EssenceLoss = SoulForceData.GainedSoulEssences * LossPercent;
    
    SoulForceData.GainedSoulEssences -= EssenceLoss;
    
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;
    
    //Lets undo stat bonuses
    
    for (const FSoulForceEffect& Effect : SoulForceData.Effects)
    {
        const float StatDelta = EssenceLoss * Effect.StatScalarValue;
        
        ModifyStat(OwnerActor, Effect.StatToScale, Effect.StatValueTypeToScale, -StatDelta);
    }
    OnSoulForceChanged.Broadcast(SoulForceData.GainedSoulEssences);
}
float UStatSystem::GetEssenceLossPercent() const
{
    if (SoulForceData.EssenceLossPercentCurve)
    {
        return SoulForceData.EssenceLossPercentCurve
            ->GetFloatValue(SoulForceData.SoulForceLevel);
    }

    return 0.f; // No penalty if curve missing
}
void UStatSystem::ScaleStats(float EssenceDelta)
{
    if (EssenceDelta <= 0.f) return;
    
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return;
    }
    
    for (const FSoulForceEffect& Effect : SoulForceData.Effects)
    {
        const float StatDelta = EssenceDelta * Effect.StatScalarValue;
        
        ModifyStat(OwnerActor, Effect.StatToScale, Effect.StatValueTypeToScale, StatDelta);
    }
}

float UStatSystem::GetLevelValueSafe(const TArray<float>& Array, int32 Level, float DefaultValue)
{
   if (Array.Num() == 0) return DefaultValue;
    
    const int32 Index = FMath::Clamp(Level, 0, Array.Num() - 1);
    return Array[Index];
}

void UStatSystem::ModifyStat(AActor* SourceModifier, const FGameplayTag Tag, const EStatValueType Type, const float Delta)
{
    for (FStatData& Stat : Stats)
    {
        if (Stat.StatTag != Tag)
            continue;

        // ---------- APPLY DELTA ----------
        switch (Type)
        {
        case EStatValueType::BaseValue:    Stat.BaseValue  += Delta; break;
        case EStatValueType::BonusValue:   Stat.BonusValue += Delta; break;
        case EStatValueType::BaseMaxValue: Stat.BaseMaxValue  += Delta; break;
        case EStatValueType::BonusMaxValue:Stat.BonusMaxValue += Delta; break;
        default: return;
        }

        // ---------- CLAMP MAX VALUES ----------
        Stat.BaseMaxValue  = FMath::Max(1.f, Stat.BaseMaxValue);
        Stat.BonusMaxValue = FMath::Max(0.f, Stat.BonusMaxValue);

        const float CurrentMaxValue = Stat.BaseMaxValue + Stat.BonusMaxValue;

        // ---------- CLAMP BASE / BONUS VALUES ----------
        Stat.BaseValue  = FMath::Clamp(Stat.BaseValue, 0.f, CurrentMaxValue);
        Stat.BonusValue = FMath::Clamp(Stat.BonusValue, 0.f, CurrentMaxValue - Stat.BaseValue);

        // ---------- FINAL DERIVED VALUES ----------
        Stat.CurrentMaxValue = CurrentMaxValue;
        Stat.CurrentValue    = Stat.BaseValue + Stat.BonusValue;

        // ---------- BROADCAST ----------
        FStatChangedEvent Event;
        Event.StatTag = Tag;
        Event.ChangeSource = SourceModifier;
        OnStatChanged.Broadcast(Event);

        // ---------- GAMEPLAY CONSEQUENCES (DEATH/REVIVAL) ----------
        if (Tag == FGameplayTag::RequestGameplayTag("Stat.Health"))
        {
            if (!bIsAlive && Stat.CurrentValue > 0.f)
            {
                bIsAlive = true;
                // Trigger OnRevived event here if needed
            }
            else if (bIsAlive && Stat.CurrentValue <= 0.f)
            {
                bIsAlive = false;
                // Trigger OnDeath event here if needed
            }
        }

        // Finished processing this stat
        return;
    }
}

bool UStatSystem::TakeDamage(AActor* DamageDealer, FVector ImpactPoint, float DamageAmount, bool& bOutKilled, float& OutDamageTaken)
{
    bOutKilled = false;
    
    if (IsDamageImmune || DamageAmount <= 0.f || !bIsAlive) { return false; }

    const FGameplayTag HealthTag = FGameplayTag::RequestGameplayTag("Stat.Health");
    const FGameplayTag ArmorTag  = FGameplayTag::RequestGameplayTag("Stat.Armor");

    float DamageAfterArmor = FMath::Max(0.f, DamageAmount - GetStatValue(ArmorTag, EStatValueType::Value));

    // Clean. Simple. Perfect.
    ModifyStat(DamageDealer, HealthTag, EStatValueType::BaseValue, -DamageAfterArmor);
    
    // Check for death if health reaches 0
    if (GetStatValue(HealthTag, EStatValueType::Value) <= 0.f)
    {
        bIsAlive = false;

        FOnDeathEvent DeathEvent;
        DeathEvent.Killer = DamageDealer;
        DeathEvent.Damage = DamageAfterArmor;
        
        OnDeath.Broadcast(DeathEvent);
        
        bOutKilled = true;
    }
    OutDamageTaken = DamageAfterArmor;
    return true;
}


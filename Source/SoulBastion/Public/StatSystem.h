#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulBastion/Data/MyStructs.h"
#include "StatSystem.generated.h"

// -------------------------
// Event payload for stat changes
// -------------------------
USTRUCT(BlueprintType)
struct FStatChangedEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FGameplayTag StatTag;        // Which stat changed
    

    UPROPERTY(BlueprintReadOnly)
    float ChangeValue;            // Amount of change applied

    UPROPERTY(BlueprintReadOnly)
    AActor* SourceModifier;       // Who caused the change

    FStatChangedEvent()
        : StatTag(), ChangeValue(0.f), SourceModifier(nullptr)
    {}
};

// -------------------------
// Event payload for death
// -------------------------
USTRUCT(BlueprintType)
struct FOnDeathEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* Killer;          // Who caused death

    UPROPERTY(BlueprintReadOnly)
    float Damage;            // Killing blow value

    FOnDeathEvent()
        : Killer(nullptr), Damage(0.f)
    {}
};

// -------------------------
// Delegates
// -------------------------
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChanged, FStatChangedEvent, StatEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, FOnDeathEvent, DeathEvent);

class UNiagaraSystem;
UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class SOULBASTION_API UStatSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStatSystem();

protected:
    virtual void BeginPlay() override;
    
    UPROPERTY(BlueprintReadOnly, Category="Stats")
    bool bInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category="Stats")
    bool bIsResting = false;

    FTimerHandle RecoveryTimerHandle;
    FTimerHandle CombatTimeoutHandle;
    
    UFUNCTION()
    void HandleStatChanged(FStatChangedEvent StatEvent);

    void EnterCombat();
    void LeaveCombat();

    float ComputeRecoveryMultiplier() const;

public:
    // -------------------------
    // Replicated stats array
    // -------------------------
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
    TArray<FStatData> Stats;
    
    
    // -------------------------
    // Getter: return Value, MaxValue and bIsAlive
    // -------------------------
    UFUNCTION(BlueprintCallable, Category="Stat System | Getter")
    void GetStatValues(FGameplayTag Tag, float& OutValue, float& OutMaxValue) const;
    
    UFUNCTION(BlueprintPure, Category="Stats")
    bool IsAlive() const { return bIsAlive; }
    // -------------------------
    // Modify only the stat's Value
    // -------------------------
    UFUNCTION(BlueprintCallable, Category="Stats")
    void ModifyStatValue(FGameplayTag Tag, float DeltaValue, AActor* SourceModifier = nullptr);

    // -------------------------
    // Modify only the stat's MaxValue
    // -------------------------
    UFUNCTION(BlueprintCallable, Category="Stats")
    void ModifyStatMaxValue(FGameplayTag Tag, float DeltaMaxValue, AActor* SourceModifier = nullptr);
    
    UFUNCTION()
    void Recovery();
    
    // Toggle Rest mode (Blueprint callable)
    UFUNCTION(BlueprintCallable, Category="Stats")
    void SetResting(bool bRest);
    
    UFUNCTION(BlueprintCallable, Category="Stats")
    bool TakeDamage(AActor* DamageDealer, FVector ImpactPoint, float DamageAmount);
    
    UFUNCTION(BlueprintPure, Category="Stats")
    float GetStatCurrentValue(FGameplayTag Stat) const;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
    TArray<UAnimMontage*> HitReactions;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
    USoundBase* HurtSound = nullptr;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
    UNiagaraSystem* HurtEffect = nullptr;
    
    // -------------------------
    // Delegates
    // -------------------------
    UPROPERTY(BlueprintAssignable, Category="Stats")
    FOnStatChanged OnStatChanged;

    UPROPERTY(BlueprintAssignable, Category="Stats")
    FOnDeath OnDeath;
    
    bool bIsAlive = true;
    protected:
    
    UPROPERTY(BlueprintReadOnly)
    FGameplayTag Status;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Stats")
    bool IsDamageImmune = false;
    
    
};

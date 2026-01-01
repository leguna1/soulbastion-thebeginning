#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulBastion/Data/MyStructs.h"
#include "AbilitySystem.h"
#include "UtilityBox.h"
#include "StatSystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChanged, FStatChangedEvent, StatEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, FOnDeathEvent, DeathEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoulForceChanged, float, NewValue);

class UNiagaraSystem;
class UAbilitySystem;
class UUtilityBox;

UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class SOULBASTION_API UStatSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStatSystem();

protected:
    virtual void BeginPlay() override;
    
    UPROPERTY(BlueprintReadOnly, Category="Stats")
    UAbilitySystem* AbilitySystem = nullptr;
    
    UPROPERTY()
    UUtilityBox* UtilityBox = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats | Soul Force")
    FSoulForceData SoulForceData;

public:
    
    //Stats array
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
    TArray<FStatData> Stats;
    
    // Getter: return Value, MaxValue and bIsAlive
    UFUNCTION(BlueprintCallable, Category="Stat System | Getter")
    float GetStatValue(FGameplayTag Tag, EStatValueType Type) const;
    
    UFUNCTION(BlueprintCallable, Category="Stat System | Getter")
    FText GetStatDetails(FGameplayTag Tag) const;

    UFUNCTION(BlueprintPure, Category="Stats")
    bool IsAlive() const { return bIsAlive; }
    
    //Setts and Modifiers    
    UFUNCTION(BlueprintCallable, Category="Stats")
    bool TakeDamage(FHitInfo InHitInfo, float& OutDamageTaken);
    
    
    UFUNCTION(BlueprintCallable, Category="Stat System | Modifier")
    void ModifyStat(AActor* SourceModifier, FGameplayTag Tag, EStatValueType Type, float Delta);
    
    
    //Soul Force
    UFUNCTION(BlueprintCallable, Category="Stat System | Modifier")
    void GainEssence(float RawEssenceAmount);
    
    UFUNCTION()
    float GetEssenceGainScalar() const;

    UFUNCTION(BlueprintCallable, Category="Stat System | Modifier")
    void SoulForceDeathPenalty();
    
    UFUNCTION()
    float GetEssenceLossPercent() const;


    // Delegates
    UPROPERTY(BlueprintAssignable, Category="Stats")
    FOnStatChanged OnStatChanged;
    
    UPROPERTY(BlueprintAssignable, Category="Stats")
    FOnSoulForceChanged OnSoulForceChanged;

    UPROPERTY(BlueprintAssignable, Category="Stats")
    FOnDeath OnDeath;
    
   
    protected:
    
    UPROPERTY(BlueprintReadOnly)
    FGameplayTag Status;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Stats")
    bool IsDamageImmune = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Stats")
    bool bIsAlive = true;
    
private:
    
    void ScaleStats(float EssenceDelta);
    static float GetLevelValueSafe(const TArray<float>& Array, int32 Level, float DefaultValue = 1.0f);
    
};

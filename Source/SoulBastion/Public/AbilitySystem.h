#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionWarpingComponent.h"
#include "SoulBastion/Data/MyStructs.h"
#include "Engine/EngineTypes.h"
#include "AbilitySystem.generated.h"

class UStatSystem;
class UPlayerCamera;
class UAbilityDataAsset;
class UNiagaraSystem;
class UCameraShakeBase;



DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillStateChanged, FGameplayTag, SkillTag, ESkillState, NewState, float, StateDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActivationInput, FGameplayTag, SkillTag, EActivationInput, Input, float, ElapsedTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMontageEventSignature, FAnimMontageData, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSkillChargeTime, FGameplayTag, SkillTag, int32, ChargeCount, float, MaxTime, float, TimeRemaining);

UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class SOULBASTION_API UAbilitySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAbilitySystem();

    // ------------------------
    // Lifecycle
    // ------------------------
protected:
    virtual void BeginPlay() override;
    void InitializeOwner();

    // ------------------------
    // References / Components
    // ------------------------
    UPROPERTY()
    ACharacter* OwnerCharacter = nullptr;
    
    UPROPERTY()
    UStatSystem* StatSystemRef = nullptr;

    UPROPERTY()
    UPlayerCamera* PlayerCameraRef = nullptr;

    UPROPERTY()
    UMotionWarpingComponent* MotionWarpingComponentRef = nullptr;
    
    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    void SetupReferences();
    void SetupEventBindings();
    void SetupMotionWarping();
    void SetupSkills();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skills")
    TArray<TSubclassOf<class USkillBase>> SkillClasses;

    
    
    UPROPERTY(Transient, BlueprintReadOnly)
    TArray<USkillBase*> SkillInstances;
    
   

    
public:
    
    UFUNCTION(BlueprintPure, Category = "Ability System")
    ACharacter* GetOwningCharacter() const { return OwnerCharacter; }
    
    UFUNCTION(BlueprintPure, Category = "Ability System")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintPure, Category = "Ability System")
    UMotionWarpingComponent* GetWarpComp() const { return MotionWarpingComponentRef; }

    UFUNCTION(BlueprintPure, Category= "Ability System")
    UStatSystem* GetOwnerStats() const { return StatSystemRef; }

    UFUNCTION(BlueprintCallable, Category="Skills")
    void TryActivateAbility(FGameplayTag SkillTag, EActivationInput Input, float InElapsedTime);
 

    UFUNCTION(BlueprintCallable, Category="Skills")
    void SetActiveSkillState(ESkillState NewState, float StateDuration);
    
    UFUNCTION(BlueprintPure, Category="Skills")
    FSkillData GetSkillData(FGameplayTag SkillTag) const;

    UPROPERTY(BlueprintAssignable, Category="Skill | Event")
    FOnSkillStateChanged OnActiveSkillStateChanged;
    
    UPROPERTY(BlueprintAssignable, Category="Skill | Event")
    FOnActivationInput OnActivationInput;
    
    UPROPERTY(BlueprintAssignable, Category= "Skill | Event")
    FMontageEventSignature OnMontageEvent;
    
    UPROPERTY(BlueprintAssignable)
    FOnSkillChargeTime OnChargeTimeUpdated;

    UPROPERTY(Transient)
    bool bBufferWindowOpen = false;

    UPROPERTY(Transient)
    USkillBase* BufferedSkill = nullptr;
    
    UPROPERTY(Transient)
    USkillBase* ActiveSkill = nullptr;
    
    UPROPERTY(Transient)
    USkillBase* OldActiveSkill = nullptr;
    
    UPROPERTY(Transient)
    TArray<FBufferedInput> BufferedInputs;

    
protected:
   
    UFUNCTION()
    void OnTargetChangedHandler(AActor* NewTarget);
    
    //Anim Montage Player
    UFUNCTION(BlueprintCallable)
    void PlayAbilityMontage(const FAbilityMontageParams& Params, bool bUseMotionWarp);
    
    UFUNCTION(BlueprintCallable)
    void ApplyMotionWarp(bool bUseWarp, const FAbilityMontageParams& MontageParams);
    UFUNCTION()
    void UpdateMontageTick(UAnimMontage* MontageRef, FAbilityMontageParams MontageParams, float PlayRateMultiplier);
    UFUNCTION()
    void UpdateMontagePlayRate(UAnimMontage* MontageRef, FAbilityMontageParams MontageParams, float PlayRateMultiplier);
    
    UFUNCTION()
    void OnMontageStartHandler(UAnimMontage* Montage);
    
    UFUNCTION()
    void OnMontageEndHandler(UAnimMontage* Montage, bool bInterrupted);
    
    UFUNCTION()
    void HandleNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

    

private:
    FTimerHandle MontageUpdateTimerHandle;
    FName CurrentWarpName;
    FName LastSectionName;
    float CurrentCurveTime = 0.f;
    
};

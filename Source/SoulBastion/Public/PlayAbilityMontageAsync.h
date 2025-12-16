#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SoulBastion/Data/MyStructs.h" // FAbilityMontageParams
#include "PlayAbilityMontageAsync.generated.h"

class ACharacter;
class UAnimMontage;
class UMotionWarpingComponent;

// Struct-based delegate solution


USTRUCT(BlueprintType)
struct FMontageAsyncData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EAnimMontageState State;

    UPROPERTY(BlueprintReadOnly)
    float NormalizedTime = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMontageAsyncDelegate, FMontageAsyncData, Data);

UCLASS()
class SOULBASTION_API UPlayAbilityMontageAsync : public UBlueprintAsyncActionBase, public FTickableGameObject
{
    GENERATED_BODY()

public:

    /** Fires all montage events using struct (Started, WhileActive, Completed, Interrupted) */
    UPROPERTY(BlueprintAssignable)
    FMontageAsyncDelegate OnMontageEvent;

public:

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
    static UPlayAbilityMontageAsync* PlayAbilityMontage(
        ACharacter* Character,
        UMotionWarpingComponent* WarpComponent,
        AActor* WarpTarget,
        UAnimMontage* Montage,
        const FAbilityMontageParams& Params,
        float SpeedMultiplier = 1.0f,
        bool bUseMotionWarp = false
    );

    virtual void Activate() override;

    /** FTickableGameObject implementation */
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UPlayAbilityMontageAsync, STATGROUP_Tickables); }
    virtual bool IsTickable() const override { return bTickable; }

private:

    /** Internal handlers */
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    /* --- Stored references --- */
    UPROPERTY()
    ACharacter* CharacterRef = nullptr;

    UPROPERTY()
    UMotionWarpingComponent* WarpCompRef = nullptr;

    UPROPERTY()
    AActor* WarpTargetRef = nullptr;

    UPROPERTY()
    UAnimMontage* MontageRef = nullptr;

    /** Cached montage settings */
    UPROPERTY()
    FAbilityMontageParams MontageParams;

    /** Speed multiplier applied on top of MontageParams.PlayRate */
    float PlayRateMultiplier = 1.f;

    /** Whether to set warp target & use motion warping */
    bool bUseWarp = false;

    /** Whether we are ticking active updates */
    bool bTickable = false;

    /** Whether montage has started */
    bool bStarted = false;

    /** Whether montage finished */
    bool bFinished = false;
};

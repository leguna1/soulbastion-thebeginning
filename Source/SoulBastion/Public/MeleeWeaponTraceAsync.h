#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "MeleeWeaponTraceAsync.generated.h"

class USkeletalMeshComponent;

USTRUCT(BlueprintType)
struct FMeleeTraceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxUniqueHits = 1; // 0 = unlimited

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHitPerActor = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.5f; // 0 = infinite
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMeleeHitDelegate, AActor*, HitActor, FVector, ImpactPoint, FVector, ImpactNormal);

UCLASS()
class SOULBASTION_API UMeleeWeaponTraceAsync : public UBlueprintAsyncActionBase, public FTickableGameObject
{
    GENERATED_BODY()

public:
    /** Fires whenever a hit occurs according to MaxUniqueHits / MaxHitPerActor */
    UPROPERTY(BlueprintAssignable)
    FMeleeHitDelegate OnHit;

    /** Start tracing from Blueprint */
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
    static UMeleeWeaponTraceAsync* StartMeleeTrace(
        USkeletalMeshComponent* WeaponMesh,
        const TArray<FName>& SocketNames,
        const FMeleeTraceSettings& Settings
    );

    /** FTickableGameObject */
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMeleeWeaponTrace, STATGROUP_Tickables); }
    virtual bool IsTickable() const override { return bIsActive; }

private:
    /** Internal start */
    virtual void Activate() override;

    /** Weapon / sockets */
    UPROPERTY()
    USkeletalMeshComponent* WeaponMeshRef = nullptr;

    UPROPERTY()
    TArray<FName> SocketNames;

    /** Trace settings */
    FMeleeTraceSettings TraceSettings;

    /** Previous frame locations */
    TArray<FVector> PrevSocketLocations;

    /** Hit tracking */
    TMap<TWeakObjectPtr<AActor>, int32> ActorHitCounts;

    /** Whether the trace is active */
    bool bIsActive = false;

    /** Time elapsed since trace started */
    float ElapsedTime = 0.f;
};

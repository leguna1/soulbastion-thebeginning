#include "PlayAbilityMontageAsync.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetMathLibrary.h"

UPlayAbilityMontageAsync* UPlayAbilityMontageAsync::PlayAbilityMontage(
    ACharacter* Character,
    UMotionWarpingComponent* WarpComponent,
    AActor* WarpTarget,
    UAnimMontage* Montage,
    const FAbilityMontageParams& Params,
    float SpeedMultiplier,
    bool bUseMotionWarp
)
{
    UPlayAbilityMontageAsync* Node = NewObject<UPlayAbilityMontageAsync>();
    Node->CharacterRef = Character;
    Node->WarpCompRef = WarpComponent;
    Node->WarpTargetRef = WarpTarget;
    Node->MontageRef = Montage;
    Node->MontageParams = Params;
    Node->PlayRateMultiplier = FMath::Max(SpeedMultiplier, 0.01f);
    Node->bUseWarp = bUseMotionWarp;
    Node->bTickable = true; // Start ticking immediately
    return Node;
}

void UPlayAbilityMontageAsync::Activate()
{
    if (!CharacterRef || !MontageRef)
    {
        FMontageAsyncData Data;
        Data.State = EAnimMontageState::Interrupted;
        OnMontageEvent.Broadcast(Data);
        SetReadyToDestroy();
        return;
    }

    UAnimInstance* AnimInstance = CharacterRef->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        FMontageAsyncData Data;
        Data.State = EAnimMontageState::Interrupted;
        OnMontageEvent.Broadcast(Data);
        SetReadyToDestroy();
        return;
    }
   
    UAnimMontage* MontageCopy = MontageRef;
  
    // Motion Warp
    if (bUseWarp && WarpCompRef && IsValid(WarpTargetRef))
    {
        const FVector InstLoc = CharacterRef->GetActorLocation();
        const FVector TargetLoc = WarpTargetRef->GetActorLocation();

        const FVector Direction = (TargetLoc - InstLoc).GetSafeNormal();
        float DistanceToTarget = FVector::Dist(InstLoc, TargetLoc);

        // Compute the delta we need to move so that final distance = IdealRange
        float DeltaDistance = DistanceToTarget - MontageParams.IdealRange;

        // Clamp delta by MaxWarpDistance (can be negative to move backward)
        float AppliedDistance = FMath::Clamp(DeltaDistance, -MontageParams.MaxWarpDistance, MontageParams.MaxWarpDistance);

        FVector WarpLoc = InstLoc + Direction * AppliedDistance;

       // FRotator WarpRot = MontageParams.bUseWarpRotation ? (TargetLoc - InstLoc).Rotation() : CharacterRef->GetActorRotation();

       // WarpCompRef->AddOrUpdateWarpTargetFromLocationAndRotation(MontageParams.WarpName, WarpLoc, WarpRot);
    }

    // Bind montage end
    AnimInstance->OnMontageEnded.AddDynamic(this, &UPlayAbilityMontageAsync::OnMontageEnded);

    // Play montage
    // Normalized time using montage play length
    float CurrentPos = AnimInstance->Montage_GetPosition(MontageCopy);
    float PlayDuration = MontageCopy->GetPlayLength();
    
    float NormalizedTime = (PlayDuration > 0.f) ? FMath::Clamp(CurrentPos / PlayDuration, 0.f, 1.f) : 0.f;

    
    float CurveRate = MontageParams.PlayRateCurve->GetFloatValue(NormalizedTime);
    
    float FinalRate = MontageParams.PlayRate * CurveRate * (1 + PlayRateMultiplier);
    
    AnimInstance->Montage_Play(MontageRef, FinalRate);
    
    if (MontageParams.StartSection != NAME_None)
    {
        AnimInstance->Montage_JumpToSection(MontageParams.StartSection, MontageRef);
    }

    // Broadcast Started event immediately
    FMontageAsyncData StartedData;
    StartedData.State = EAnimMontageState::Started;
    StartedData.NormalizedTime = 0.f;
    OnMontageEvent.Broadcast(StartedData);

    bStarted = true;
}

void UPlayAbilityMontageAsync::Tick(float DeltaTime)
{
    if (!bStarted || bFinished || !CharacterRef || !MontageRef)
        return;

    UAnimInstance* AnimInstance = CharacterRef->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
        return;

    if (!AnimInstance->Montage_IsPlaying(MontageRef))
        return;

    float Pos = AnimInstance->Montage_GetPosition(MontageRef);
    float Len = MontageRef->GetPlayLength();
    float NormalizedTime = (Len > 0.f) ? FMath::Clamp(Pos / Len, 0.f, 1.f) : 0.f;

    float CurveRate = MontageParams.PlayRateCurve 
        ? MontageParams.PlayRateCurve->GetFloatValue(NormalizedTime) 
        : 1.f;

    float FinalRate = MontageParams.PlayRate * CurveRate * (1 + PlayRateMultiplier);

    AnimInstance->Montage_SetPlayRate(MontageRef, FinalRate);

    FMontageAsyncData Data;
    Data.State = EAnimMontageState::WhileActive;
    Data.NormalizedTime = NormalizedTime;
    OnMontageEvent.Broadcast(Data);
}

void UPlayAbilityMontageAsync::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!CharacterRef)
        return;

    bFinished = true;
    bTickable = false;
    if (IsValid(WarpTargetRef))
    {
        WarpCompRef->RemoveWarpTarget(MontageParams.WarpName);
    }

    FMontageAsyncData Data;
    Data.State = bInterrupted ? EAnimMontageState::Interrupted : EAnimMontageState::Completed;
    Data.NormalizedTime = 1.f;
    OnMontageEvent.Broadcast(Data);

    SetReadyToDestroy();
}
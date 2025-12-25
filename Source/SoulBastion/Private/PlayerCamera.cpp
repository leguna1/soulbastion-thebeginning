#include "PlayerCamera.h"
#include "StatSystem.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"

UPlayerCamera::UPlayerCamera()
{
    PrimaryComponentTick.bCanEverTick = true;
}
void UPlayerCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bDebug || !CurrentTarget || !IsActorAlive(CurrentTarget)) return;

    // Draw persistent debug sphere on current target
    DrawDebugSphere(GetWorld(), CurrentTarget->GetActorLocation(), 30.f, 16, FColor::Yellow, false, -1.f, 0, 1.5f);

    // Optional: line from camera to target
    DrawDebugLine(GetWorld(), GetComponentLocation(), CurrentTarget->GetActorLocation(), FColor::Yellow, false, -1.f, 0, 0.2f);
}
void UPlayerCamera::BeginPlay()
{
    Super::BeginPlay();

    // Auto-target update every 0.2s
    GetWorld()->GetTimerManager().SetTimer(TargetUpdateTimer, this, &UPlayerCamera::UpdateAutoTarget, 0.2f, true);
}

void UPlayerCamera::UpdateAutoTarget()
{
    // Keep current target if still alive and in range
    if (CurrentTarget && IsActorAlive(CurrentTarget))
    {
        float Dist = FVector::Dist(CurrentTarget->GetActorLocation(), GetComponentLocation());
        if (Dist <= TargetRadius)
            return; // Still valid
    }

    // Find new target
    AActor* NewTarget = FindBestTarget();

    if (NewTarget != CurrentTarget)
    {
        CurrentTarget = NewTarget;
        OnTargetChanged.Broadcast(CurrentTarget);
    }
    
}

AActor* UPlayerCamera::FindBestTarget() const
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return nullptr;

    FVector CamLocation = GetComponentLocation();
    FVector CamForward = GetForwardVector();

    AActor* BestTarget = nullptr;
    float ClosestDist = TargetRadius;

    for (TActorIterator<AActor> It(GetWorld(), TargetActorClass); It; ++It)
    {
        AActor* Candidate = *It;
        if (!Candidate || Candidate == OwnerActor) continue;
        if (!IsActorAlive(Candidate)) continue;

        float Dist = FVector::Dist(Candidate->GetActorLocation(), CamLocation);
        if (Dist > TargetRadius) continue;

        FVector ToTarget = (Candidate->GetActorLocation() - CamLocation).GetSafeNormal();
        float Dot = FVector::DotProduct(CamForward, ToTarget);
        if (Dot < FMath::Cos(FMath::DegreesToRadians(FOVHalfAngle))) continue;

        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            BestTarget = Candidate;
        }
    }

    return BestTarget;
}

bool UPlayerCamera::IsActorAlive(const AActor* Actor)
{
    if (!Actor) return false;

    if (UStatSystem* StatComp = Actor->FindComponentByClass<UStatSystem>())
    {
        return StatComp->IsAlive();
    }

    return false;
}

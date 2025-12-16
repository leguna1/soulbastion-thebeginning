#include "MeleeWeaponTraceAsync.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"

UMeleeWeaponTraceAsync* UMeleeWeaponTraceAsync::StartMeleeTrace(
    USkeletalMeshComponent* WeaponMesh,
    const TArray<FName>& InSocketNames,
    const FMeleeTraceSettings& Settings
)
{
    if (!WeaponMesh || InSocketNames.Num() < 2)
        return nullptr;

    UMeleeWeaponTraceAsync* Node = NewObject<UMeleeWeaponTraceAsync>();
    Node->WeaponMeshRef = WeaponMesh;
    Node->SocketNames = InSocketNames;
    Node->TraceSettings = Settings;
    Node->Activate();
    return Node;
}

void UMeleeWeaponTraceAsync::Activate()
{
    if (!WeaponMeshRef || SocketNames.Num() < 2)
        return;

    // Initialize previous locations
    PrevSocketLocations.SetNum(SocketNames.Num());
    for (int32 i = 0; i < SocketNames.Num(); ++i)
    {
        PrevSocketLocations[i] = WeaponMeshRef->GetSocketLocation(SocketNames[i]);
    }

    ActorHitCounts.Empty();
    bIsActive = true;
    ElapsedTime = 0.f;
}

void UMeleeWeaponTraceAsync::Tick(float DeltaTime)
{
    if (!bIsActive || !WeaponMeshRef || SocketNames.Num() < 2)
        return;

    // Update elapsed time
    ElapsedTime += DeltaTime;
    if (TraceSettings.Duration > 0.f && ElapsedTime >= TraceSettings.Duration)
    {
        bIsActive = false;
        return;
    }

    UWorld* World = WeaponMeshRef->GetWorld();
    if (!World)
        return;

    const int32 NumSockets = SocketNames.Num();
    TArray<FVector> CurrLocations;
    CurrLocations.SetNum(NumSockets);

    for (int32 i = 0; i < NumSockets; ++i)
        CurrLocations[i] = WeaponMeshRef->GetSocketLocation(SocketNames[i]);

    // Trace segments
    for (int32 i = 0; i < NumSockets; ++i)
    {
        int32 j = (i + 1) % NumSockets;
        const FVector Start = PrevSocketLocations[i];
        const FVector End = CurrLocations[j];

        
    
        FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeTrace), true);
        if (AActor* Owner = WeaponMeshRef->GetOwner())
            Params.AddIgnoredActor(Owner);

        FCollisionObjectQueryParams ObjectQueryParams(TraceSettings.TraceChannel);
        
        TArray<FHitResult> HitResults;
        World->LineTraceMultiByObjectType(HitResults, Start, End, ObjectQueryParams, Params);
        if (TraceSettings.bDebug)
            DrawDebugLine(World, Start, End, FColor::Green, false, 0.1f, 0, 1.5f);

        HitResults.StableSort([Start](const FHitResult& A, const FHitResult& B)
        {
            return (A.ImpactPoint - Start).SizeSquared() < (B.ImpactPoint - Start).SizeSquared();
        });

        for (const FHitResult& HR : HitResults)
        {
            AActor* HitActor = HR.GetActor();
            if (!HitActor)
                continue;

            int32* CountPtr = ActorHitCounts.Find(HitActor);
            if (!CountPtr)
            {
                if (TraceSettings.MaxUniqueHits > 0 && ActorHitCounts.Num() >= TraceSettings.MaxUniqueHits)
                    continue;

                ActorHitCounts.Add(HitActor, 1);
                OnHit.Broadcast(HitActor, HR.ImpactPoint, HR.ImpactNormal);
            }
            else if (*CountPtr < TraceSettings.MaxHitPerActor)
            {
                (*CountPtr)++;
                OnHit.Broadcast(HitActor, HR.ImpactPoint, HR.ImpactNormal);
            }
        }
    }

    PrevSocketLocations = MoveTemp(CurrLocations);
}

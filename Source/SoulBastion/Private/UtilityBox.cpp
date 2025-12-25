// Fill out your copyright notice in the Description page of Project Settings.


#include "UtilityBox.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"


// Sets default values for this component's properties
UUtilityBox::UUtilityBox()
{
	
	PrimaryComponentTick.bCanEverTick = false;

	
}

void UUtilityBox::SpawnHitEffects(FVector ImpactPoint, FVector SurfaceNormal, FFxData FxData) //Keep
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	//SpawnVFX
	if (FxData.NiagaraSystem)
	{
		FVector AdjustedLocation = ImpactPoint + SurfaceNormal * FxData.EffectOffset;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			FxData.NiagaraSystem,
			AdjustedLocation,
			SurfaceNormal.Rotation()
		);
	}
	// --- Spawn SFX ---
	if (FxData.SoundToPlay)
	{
		FVector SoundLocation = ImpactPoint;
		UGameplayStatics::PlaySoundAtLocation(World, FxData.SoundToPlay, SoundLocation, FxData.SoundVolume, FxData.SoundPitch);
	}
}

void UUtilityBox::PlayMyCameraShake(const FCameraShakeData& CameraShakeSetting) //Keep
{
	if (!CameraShakeSetting.ShakeClass) return;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UWorld* World = OwnerActor->GetWorld();
	if (!World) return;

	
	UGameplayStatics::PlayWorldCameraShake(
		World,
		CameraShakeSetting.ShakeClass,
		CameraShakeSetting.Epicenter,
		CameraShakeSetting.InnerRadius,
		CameraShakeSetting.OuterRadius,
		CameraShakeSetting.Falloff,
		CameraShakeSetting.bOrientShakeTowardsEpicenter
	);
}

int32 UUtilityBox::GetHitReactionIndex(const FVector& ImpactPoint) const //Keep
{
	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const FRotator OwnerRotation = GetOwner()->GetActorRotation();
    
	//World direction from owner -> hit point, normalize.
	FVector Dir = ImpactPoint - OwnerLocation;
	Dir.Normalize();
    
	//Convert World -> Local Space
	const FVector LocalDir = OwnerRotation.UnrotateVector(Dir);
    
	const float Forward = LocalDir.X;
	const float Right = LocalDir.Y;
    
	//Determine the quadrant 

	if (FMath::Abs(Forward) > FMath::Abs(Right))
	{
		return (Forward >= 0.f) ? 0 : 1; //0= Front, 1= Back
	}
	else
	{
		return (Right >= 0.f) ? 3 : 2; //3 = Right, 2 = Left
	}
}

void UUtilityBox::GetRelativeDirectionAndDistance(const AActor* SelfActor, const AActor* TargetActor, int32& OutDirectionIndex,
	float& OutDistance)
{
	OutDirectionIndex = -1;
	OutDistance = 0.f;

	if (!SelfActor || !TargetActor) return;

	const FVector SelfLoc = SelfActor->GetActorLocation();
	const FVector TargetLoc = TargetActor->GetActorLocation();
	const FVector ToTarget = TargetLoc - SelfLoc;

	OutDistance = ToTarget.Size();

	FVector ToTargetNorm = ToTarget.GetSafeNormal();

	FVector Forward = SelfActor->GetActorForwardVector().GetSafeNormal2D();
	FVector Right   = SelfActor->GetActorRightVector().GetSafeNormal2D();

	float ForwardDot = FVector::DotProduct(Forward, ToTargetNorm);
	float RightDot   = FVector::DotProduct(Right,   ToTargetNorm);

	// Correct signed angle
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

	// Normalize angle to 0–360
	AngleDegrees = FMath::Fmod(AngleDegrees + 360.f, 360.f);

	// 45° per direction
	OutDirectionIndex = FMath::FloorToInt(AngleDegrees / 45.f);

	// Safety clamp 0–7
	OutDirectionIndex = FMath::Clamp(OutDirectionIndex, 0, 7);
	
}
int32 UUtilityBox::GetSideDirection(const AActor* SelfActor, const AActor* TargetActor, const bool bFwdBwdCountsAsSide,float& OutDistance)

{
	OutDistance = 0.f;

	if (!SelfActor || !TargetActor)
		return FMath::RandRange(0, 2);

	const FVector SelfLoc = SelfActor->GetActorLocation();
	const FVector TargetLoc = TargetActor->GetActorLocation();
	const FVector ToTarget = TargetLoc - SelfLoc;
	OutDistance = ToTarget.Size();

	const FVector ToTargetNorm = ToTarget.GetSafeNormal();

	const FVector Forward = SelfActor->GetActorForwardVector().GetSafeNormal2D();
	const FVector Right   = SelfActor->GetActorRightVector().GetSafeNormal2D();

	const float ForwardDot = FVector::DotProduct(Forward, ToTargetNorm);
	const float RightDot   = FVector::DotProduct(Right,   ToTargetNorm);

	float AngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

	// Normalize to 0–360
	AngleDegrees = FMath::Fmod(AngleDegrees + 360.f, 360.f);

	// Octants: 45° each

	/*
		Octant Map:
		0 = Forward
		1 = Forward-Right
		2 = Right
		3 = Back-Right
		4 = Back
		5 = Back-Left
		6 = Left
		7 = Forward-Left
	*/

	// Handle left/right groups
	switch (const int32 Octant = FMath::FloorToInt(AngleDegrees / 45.f) % 8)
	{
	case 1: // FR
	case 2: // R
	case 3: // BR
		return 2; // RIGHT SIDE

	case 7: // FL
	case 6: // L
	case 5: // BL
		return 1; // LEFT SIDE
	default: ;
	}

	// Forward (0) or Backward (4)
	if (!bFwdBwdCountsAsSide)
	{
		// If NOT counting as side → return 0
		return 0;
	}

	// If counting forward/backward as side:
	// Use the side based on the right vector dot
	if (RightDot >= 0.f)
		return 2; // Forward/Back but leaning right
	else
		return 1; // Forward/Back but leaning left
}

int32 UUtilityBox::SelectMontageIndex(const AActor* Instigator, const AActor* Target, const TArray<FMontageEntry>& Data)
{
    const int32 ExpectedSize = 12;
    if (Data.Num() != ExpectedSize)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SelectMontageIndex: MontageData MUST have %d elements (has %d)"),
            ExpectedSize, Data.Num());
        return FMath::RandRange(0, Data.Num() - 1);
    }

    // Fallback random if no actors
    if (!Instigator || !Target)
    {
        return FMath::RandRange(0, Data.Num() - 1);
    }

    const FVector InstLoc = Instigator->GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();
    const FVector ToTarget = (TargetLoc - InstLoc).GetSafeNormal();

    const FVector Forward = Instigator->GetActorForwardVector();
    const FVector Right   = Instigator->GetActorRightVector();

    // Determine Direction (0 = Front, 1 = Right, 2 = Left, 3 = Back)
    const float F = FVector::DotProduct(Forward, ToTarget);
    const float R = FVector::DotProduct(Right, ToTarget);

    int32 Direction = 0;
    if (F > 0.85f)           Direction = 0; // Front
    else if (F < -0.85f)     Direction = 3; // Back
    else if (R > 0.0f)       Direction = 1; // Right
    else                     Direction = 2; // Left

	// Distance to target
	const float Distance = FVector::Distance(InstLoc, TargetLoc);

	// -------------------------------------------
	// Smart Reach Selection based on distance to target and max reach range.
	// -------------------------------------------

	// These 3 entries correspond to this direction: dir + (0,4,8)
	TArray<int32> Candidates;
	Candidates.Add(Direction + 0 * 4);
	Candidates.Add(Direction + 1 * 4);
	Candidates.Add(Direction + 2 * 4);

	float BestScore = FLT_MAX;
	int32 BestIndex = Candidates[0];

	const float UnderShootPenalty = 1000.0f;    // Heavy bias against missing
	const float OverShootFactor   = 0.1f;       // Light penalty for big overshoot

	// Band weights
	auto GetBandBias = [&](float Reach)
	{
		if (Reach < 200.0f)  return 1.0f;  // Close priority
		if (Reach < 350.0f)  return 0.8f;  // Mid-priority
		return 0.6f;                       // Long priority
	};

	for (int32 Index : Candidates)
	{
		const FMontageEntry& E = Data[Index];

		float Reach = E.IdealRange + E.MaxTranslationDistance;
		float Error = FMath::Abs(Reach - Distance);

		// Undershoot (cannot reach)
		if (Reach < Distance)
			Error += UnderShootPenalty;

		// Overshoot (movement too much forward)
		float Over = Reach - Distance;
		if (Over > 0)
			Error += Over * OverShootFactor;

		// Apply band preference weight
		float BandBias = GetBandBias(Reach);
		Error *= BandBias;

		if (Error < BestScore)
		{
			BestScore = Error;
			BestIndex = Index;
		}
	}

	// Safety
	if (!Data[BestIndex].Montage)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SelectMontageIndex: Montage missing at index %d (dir=%d)"),
			BestIndex, Direction);
	}

	return BestIndex;
}
int32 UUtilityBox::SelectMontageIndex_NoBack(const AActor* Instigator, const AActor* Target, const TArray<FMontageEntry>& Data) //Rethink
{
	constexpr int32 ExpectedSize = 9;
    if (Data.Num() != ExpectedSize)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SelectMontageIndex_NoBack_9: MontageData MUST have %d elements (has %d)"),
            ExpectedSize, Data.Num());

        return FMath::RandRange(0, Data.Num() - 1);
    }

    // -------------------------------------------------
    // Safety: missing actors -> random
    // -------------------------------------------------
    if (!Instigator || !Target)
    {
        return FMath::RandRange(0, Data.Num() - 1);
    }

    const FVector InstLoc   = Instigator->GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();
    const FVector ToTarget  = (TargetLoc - InstLoc).GetSafeNormal();

    const FVector Forward = Instigator->GetActorForwardVector();
    const FVector Right   = Instigator->GetActorRightVector();

    // -------------------------------------------------
    // Direction (0 = Front, 1 = Right, 2 = Left)
    // Back hemisphere folds into Left / Right
    // -------------------------------------------------
    const float F = FVector::DotProduct(Forward, ToTarget);
    const float R = FVector::DotProduct(Right,   ToTarget);

    int32 Direction = 0;

    if (F > 0.85f)
    {
        Direction = 0; // Front
    }
    else
    {
        // Side or Back -> decide by tilt
        Direction = (R >= 0.0f) ? 1 : 2; // Right / Left
    }
	// -------------------------------------------------
	// Optional randomness for pure back case
	// -------------------------------------------------
	if (F < -0.9f && FMath::Abs(R) < 0.15f)
	{
		Direction = FMath::RandBool() ? 1 : 2; // Right or Left
	}
    // -------------------------------------------------
    // Distance to target
    // -------------------------------------------------
    const float Distance = FVector::Distance(InstLoc, TargetLoc);

    // -------------------------------------------------
    // Candidates for this direction:
    // dir + {0, 3, 6}
    // -------------------------------------------------
    TArray<int32> Candidates;
    Candidates.Reserve(3);
    Candidates.Add(Direction + 0 * 3);
    Candidates.Add(Direction + 1 * 3);
    Candidates.Add(Direction + 2 * 3);

    float BestScore = FLT_MAX;
    int32 BestIndex = Candidates[0];

	// Band preference weighting (numeric, no enum)
    auto GetBandBias = [&](float Reach)
    {
        if (Reach < 200.0f)  return 1.0f;  // Close
        if (Reach < 350.0f)  return 0.8f;  // Mid
        return 0.6f;                       // Long
    };

    // -------------------------------------------------
    // Smart reach selection
    // -------------------------------------------------
    for (int32 Index : Candidates)
    {
	    constexpr float OverShootFactor   = 0.1f;
	    constexpr float UnderShootPenalty = 1000.0f;
	    const FMontageEntry& E = Data[Index];

        const float Reach = E.IdealRange + E.MaxTranslationDistance;
        float Score = FMath::Abs(Reach - Distance);

        // Undershoot: cannot reach target → very bad
        if (Reach < Distance)
            Score += UnderShootPenalty;

        // Overshoot: too much movement → mild penalty
        const float Over = Reach - Distance;
        if (Over > 0.0f)
            Score += Over * OverShootFactor;

        // Apply band preference
        Score *= GetBandBias(Reach);

        if (Score < BestScore)
        {
            BestScore = Score;
            BestIndex = Index;
        }
    }

    // -------------------------------------------------
    // Safety / debug
    // -------------------------------------------------
    if (!Data.IsValidIndex(BestIndex))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SelectMontageIndex_NoBack_9: Invalid index %d"), BestIndex);

        return FMath::RandRange(0, Data.Num() - 1);
    }

    if (!Data[BestIndex].Montage)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SelectMontageIndex_NoBack_9: Montage missing at index %d (dir=%d)"),
            BestIndex, Direction);
    }

    return BestIndex;
}

// Called when the game starts
void UUtilityBox::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UUtilityBox::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
void UUtilityBox::SetIgnoreOtherCharacters(ACharacter* Character, const ECollisionChannel Channel, const bool bIgnore)
{
	if (!Character) return;
	
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	if (!Capsule) return;
	
	Capsule->SetCollisionResponseToChannel(Channel, bIgnore ? ECR_Ignore : ECR_Block);
} //Keep
bool UUtilityBox::HasPC() const
{
	const APawn* PawnObject = Cast<APawn>(GetOwner());
	if (!PawnObject)
		return false;

	const AController* Controller = PawnObject->GetController();
	if (!Controller)
		return false;

	return Controller->IsA(APlayerController::StaticClass());
} //Keep
int32 UUtilityBox::GetDirectionalIndex(const AActor* Target) const
{
	const AActor* Owner = GetOwner();
	
	if (!Owner || !Target) return 0;
	
	const FVector Forward = Owner->GetActorForwardVector();
	const FVector ToTarget = (Target->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
	
	const float Dot = FVector::DotProduct(Forward, ToTarget);
	const FVector Cross = FVector::CrossProduct(Forward, ToTarget);
	
	if (Dot > 0.9) return 0;
	
	return (Cross.Z > 0.f) ? 2 : 1; // Left : Right
}

int32 UUtilityBox::GetComboMontageIndex(const AActor* TargetActor, int32 ComboCount, int32 LastIndex) const
{
	// ReSharper disable once CppTooWideScope
	constexpr int32 FinisherIndex = 3;
	
	//Force Finisher
	if (ComboCount == 3)
	{
		return FinisherIndex;
	}
	
	//Opener
	if (ComboCount == 0)
	{
		return GetDirectionalIndex(TargetActor);
	}
	
	//Anti-repetition
	
	switch (LastIndex)
	{
		case 2: return 1;
		case 1: return 2;
		case 0: return (FMath::RandBool() ? 1 : 2);
		
		default: return 1;
	}
	
}



#pragma once
#include "CoreMinimal.h"
#include "MyEnums.h"
#include "Engine/Texture2D.h" // For UTexture2D
#include "GameplayTagContainer.h"   // <-- Needed for FGameplayTag
#include "NiagaraSystem.h"
#include "Engine/EngineTypes.h"
#include "MyStructs.generated.h"

// ✅ Forward declarations
class UAbilityDataAsset;

USTRUCT(BlueprintType)
struct FStatData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StatTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 0.f;

	UPROPERTY()
	float OldValue = 0.f;

	UPROPERTY()
	float OldMaxValue = 0.f;

	// Default constructor (required for Blueprint delegates and default initialization)
	FStatData()
		: StatTag(), Value(0.f), MaxValue(0.f), OldValue(0.f), OldMaxValue(0.f) {}

	// Convenience constructor for C++ initialization
	FStatData(FGameplayTag InTag, float InValue, float InMaxValue)
		: StatTag(InTag), Value(InValue), MaxValue(InMaxValue), OldValue(InValue), OldMaxValue(InMaxValue) {}
};

USTRUCT(BlueprintType)
struct FTargetInfo
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
	bool bHasValidTarget = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
	AActor* TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
	FVector TargetLocation = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
	float Distance = 0.0f;

	FTargetInfo() = default;
};


USTRUCT(BlueprintType)
struct FSkillStateTime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillState SkillState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.f;

	UPROPERTY()
	bool bShouldAscend; //If true, TimeRemaining start from 0 to meet Time
	
};

USTRUCT(BlueprintType)
struct FAbilityMontageParams
{
	GENERATED_BODY()
	

	// Section to jump to after play
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StartSection = NAME_None;

	// Base play rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate = 1.0f;

	// Optional curve for dynamic play rate across montage (Normalized 0..1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* PlayRateCurve = nullptr;

	
	// Translation mode (None / ToTargetActor / Forward / Backward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMotionWarpTranslationMode TranslationMode = EMotionWarpTranslationMode::None;

	// Rotation mode (None / ToTargetActor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMotionWarpRotationMode RotationMode = EMotionWarpRotationMode::None;

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WarpName = FName("Warp");

	// Ideal range used to compute warp location (distance to stop before target)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdealRange = 0.0f;

	// Maximum allowed warp distance from instigator (clamping)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxWarpDistance = 0.0f;
	
};

USTRUCT(BlueprintType)
struct FBufferedInput
{
	GENERATED_BODY()
	
	UPROPERTY()
	FGameplayTag SkillTag;
	
	UPROPERTY()
	EActivationInput Input;
	
	UPROPERTY()
	float ElapsedTime; 	
};

USTRUCT(BlueprintType)
struct FAnimMontageData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	mutable EAnimMontageState State;
	
	UPROPERTY(BlueprintReadOnly)
	FName NotifyName = NAME_None;   // 🔥 new field

	UPROPERTY(BlueprintReadOnly)
	mutable float NormalizedTime = 0.f;
};
USTRUCT(BlueprintType)
struct FSkillData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	FText Name = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	FText Description = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	UTexture2D* Icon = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float EnergyCost = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float SkillLevel = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float CurrentXP = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float NextLevelXP = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float BasePower = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float BaseSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	int32 MaxCharge = 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Data | Structs")
	int32 CurrentCharge = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float RechargeTime = 0.f;
	
};

USTRUCT(BlueprintType)
struct FFxData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	UNiagaraSystem* NiagaraSystem = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float EffectOffset = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	USoundBase* SoundToPlay = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float SoundVolume = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float SoundPitch = 1.f;
	
};

USTRUCT(BlueprintType)
struct FCameraShakeData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	TSubclassOf<UCameraShakeBase> ShakeClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	FVector Epicenter = FVector::ZeroVector; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float InnerRadius = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float OuterRadius = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float Falloff = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	bool bOrientShakeTowardsEpicenter = false;
};

USTRUCT(BlueprintType)
struct FMontageEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdealRange = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTranslationDistance = 100.f;
};
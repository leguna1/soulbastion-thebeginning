#pragma once
#include "CoreMinimal.h"
#include "MyEnums.h"
#include "Engine/Texture2D.h" // For UTexture2D
#include "GameplayTagContainer.h"   // <-- Needed for FGameplayTag
#include "NiagaraSystem.h"
#include "Engine/EngineTypes.h"
#include "MyStructs.generated.h"

USTRUCT(BlueprintType)
struct FSoulForceEffect
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StatToScale = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StatScalarValue = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EStatValueType StatValueTypeToScale = EStatValueType::None;
};

USTRUCT(BlueprintType)
struct FSoulForceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StatTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description = FText::GetEmpty();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SoulForceLevel = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveFloat* EssenceGainScalarCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveFloat* EssenceLossPercentCurve = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float GainedSoulEssences = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float SavedSoulEssences = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSoulForceEffect> Effects;
	
};

USTRUCT(BlueprintType)
struct FStatData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StatTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;
	
	UPROPERTY()
	float CurrentValue = 0.f;

	UPROPERTY()
	float CurrentMaxValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseMaxValue = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BonusValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BonusMaxValue = 0.f;
	
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
	
};


USTRUCT(BlueprintType)
struct FSkillStateTime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillState SkillState = ESkillState::Ready;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.f;

	UPROPERTY()
	bool bShouldAscend = false; //If true, TimeRemaining start from 0 to meet Time
	
};

USTRUCT(BlueprintType)
struct FAbilityMontageParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* MontageToPlay = nullptr;
	
	// Section to jump to after play
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StartSection = NAME_None;

	// Play rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRateMultiplayer = 0.f;

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
	FGameplayTag SkillTag = FGameplayTag::EmptyTag;
	
	UPROPERTY()
	EActivationInput Input = EActivationInput::Tap;
	
	UPROPERTY()
	FVector2D InputAction = FVector2D::ZeroVector;
	
	UPROPERTY()
	float ElapsedTime = 0.f; 	
};

USTRUCT(BlueprintType)
struct FMyPlayerInput
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Data | Structs")
	EActivationInput Input = EActivationInput::Tap;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Data | Structs")
	FVector2D InputAction = FVector2D::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Data | Structs")
	float InElapsedTime = 0.f;
};

USTRUCT(BlueprintType)
struct FAnimMontageData
{
	GENERATED_BODY()

	
	UPROPERTY(BlueprintReadOnly)
	mutable EAnimMontageState State = EAnimMontageState::Started;
	
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
	int32 SkillLevel = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float BasePower = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	float BaseSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
	int32 MaxCharge = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data | Structs")
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
USTRUCT(BlueprintType)
struct FStatChangedEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	AActor* ChangeSource = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(BlueprintReadOnly)
	float Delta = 0.f;
	
	
};
USTRUCT(BlueprintType)
struct FHitInfo
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AActor* SourceActor = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FGameplayTag SourceTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float StateDuration = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float  DamageAmount = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector ImpactPoint = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector ImpactNormal = FVector::ZeroVector;
	
};

USTRUCT(BlueprintType)
struct FOnDeathEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	AActor* Killer = nullptr;          // Who caused death

	UPROPERTY(BlueprintReadOnly)
	float Damage = 0.f;            // Killing blow value
};

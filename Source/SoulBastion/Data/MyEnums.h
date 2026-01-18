#pragma once

#include "CoreMinimal.h"
#include "MyEnums.generated.h"

UENUM(BlueprintType)
enum class ESkillState : uint8
{
	Ready UMETA(DisplayName = "Ready"),
	Windup UMETA(DisplayName = "Windup"),
	Active UMETA(DisplayName = "Active"),
	Recovery UMETA(DisplayName = "Recovery")
};
UENUM(BlueprintType)
enum class  EActivationInput : uint8
{
	Tap UMETA(DisplayName = "Tap"),
	Pressed UMETA(DisplayName = "Pressed"),
	Released UMETA(DisplayName = "Released"),
};

UENUM(BlueprintType)
enum class EAnimMontageState : uint8
{
	Started,
	Interrupted,
	Completed,
	WhileActive,
	NotifyBegin
};

UENUM(BlueprintType)
enum class EInterruptibleState : uint8
{
	None UMETA(DisplayName = "None"),
	Any UMETA(DisplayName = "Any"),
	Windup UMETA(DisplayName = "Windup"),
	Active UMETA(DisplayName = "Active"),
	Recovery UMETA(DisplayName = "Recovery"),
	WindupOrActive UMETA(DisplayName = "Windup Or Active"),
	WindupOrRecovery UMETA(DisplayName = "Windup Or Recovery"),
	ActiveOrRecovery UMETA(DisplayName = "Active Or Recovery"),
};

UENUM(BlueprintType)
enum class EMotionWarpTranslationMode : uint8
{
	None                UMETA(DisplayName="None"),
	ToTargetActor       UMETA(DisplayName="Move To Target Actor"),
	Forward             UMETA(DisplayName="Move Forward"),
	Backward            UMETA(DisplayName="Move Backward")
};

UENUM(BlueprintType)
enum class EMotionWarpRotationMode : uint8
{
	None                UMETA(DisplayName="None"),
	ToTargetActor       UMETA(DisplayName="Rotate To Target Actor")
};
UENUM(BlueprintType)
enum class EStatValueType : uint8
{
	None UMETA(DisplayName = "None"),
	Value UMETA(DisplayName = "Value"),
	MaxValue UMETA(DisplayName = "Max Value"),
	BaseValue UMETA(DisplayName = "Base Value"),
	BaseMaxValue UMETA(DisplayName = "Base Max Value"),
	BonusValue UMETA(DisplayName = "Bonus Value"),
	BonusMaxValue UMETA(DisplayName = "Bonus Max Value")
};
UENUM(BlueprintType)
enum class EMoveMode : uint8
{
	None UMETA(DisplayName = "None"),
	Walk UMETA(DisplayName = "Walking"),
	Flying UMETA(DisplayName = "Flying"),
	Swimming UMETA(DisplayName = "Swimming"),
	Crouching UMETA(DisplayName = "Crouching"),
	Climbing UMETA(DisplayName = "Climbing"),
};
UENUM(BlueprintType)
enum class EAbilityActivateResult : uint8
{
	Success             UMETA(DisplayName="Success"),
	Failed_InvalidTag   UMETA(DisplayName="Invalid Tag"),
	Failed_NoEnergy     UMETA(DisplayName="Not Enough Energy"),
	Failed_NoCharges    UMETA(DisplayName="No Charges"),
	Failed_Other        UMETA(DisplayName="Other")
};
UENUM(BlueprintType)
enum class EHitResult : uint8
{
	None UMETA(DisplayName = "None"),
	Damaged UMETA(DisplayName="Damaged"),
	Blocked UMETA(DisplayName="Blocked"),
	Parried UMETA(DisplayName="Parried"),
	Dodged UMETA(DisplayName="Dodged"),
	Clashed UMETA(DisplayName="Clashed"),
};
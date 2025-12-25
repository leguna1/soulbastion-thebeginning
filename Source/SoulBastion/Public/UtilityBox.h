// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulBastion/Data/MyEnums.h"
#include "SoulBastion/Data/MyStructs.h"
#include "Sound/SoundBase.h"

#include "UtilityBox.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULBASTION_API UUtilityBox : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUtilityBox();
	
	/** Play impact FX with VFX + SFX data */
	UFUNCTION(BlueprintCallable, Category="UtilityBox|FX")
	void SpawnHitEffects(FVector ImpactPoint, FVector SurfaceNormal, FFxData FxData);
	

	UFUNCTION(BlueprintCallable, Category="UtilityBox|Camera")
	void PlayMyCameraShake(const FCameraShakeData& CameraShakeSetting);
	
	UFUNCTION(BlueprintPure, Category="UtilityBox|Math")
	int32 GetHitReactionIndex(const FVector& ImpactPoint) const;

	
	UFUNCTION(BlueprintPure, Category="UtilityBox|Math")
	static void GetRelativeDirectionAndDistance(const AActor* SelfActor, const AActor* TargetActor, int32& OutDirectionIndex, float& OutDistance);
	
	UFUNCTION(BlueprintPure, Category="UtilityBox|Math")
	static int32 GetSideDirection(const AActor* SelfActor, const AActor* TargetActor, bool bFwdBwdCountsAsSide,
	                              float& OutDistance);

	UFUNCTION(BlueprintCallable, Category="UtilityBox|Math")
	static int32 SelectMontageIndex(const AActor* Instigator, const AActor* Target, const TArray<FMontageEntry>& Data);
	
	UFUNCTION(BlueprintCallable, Category="UtilityBox|Math")
	static int32 SelectMontageIndex_NoBack(const AActor* Instigator, const AActor* Target, const TArray<FMontageEntry>& Data);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMontageEntry> MontageData;   // MUST have 12 elements
	
	UFUNCTION(BlueprintCallable, Category="UtilityBox|Collision")
	static void SetIgnoreOtherCharacters(ACharacter* Character, ECollisionChannel Channel, bool bIgnore);
	
	UFUNCTION(BlueprintCallable, Category="UtilityBox|Verifier")
	bool HasPC() const;
	
	UFUNCTION(BlueprintCallable, Category="UtilityBox|Math")
	int32 GetDirectionalIndex(const AActor* Target) const;
	
	UFUNCTION(BlueprintCallable, Category="UtilityBox|Math")
	int32 GetComboMontageIndex(const AActor* TargetActor, int32 ComboCount, int32 LastIndex) const;
	
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	
	
};

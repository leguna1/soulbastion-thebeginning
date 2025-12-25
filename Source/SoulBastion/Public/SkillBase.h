// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem.h"
#include "SoulBastion/Data/MyEnums.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "SoulBastion/Data/MyDebug.h"
#include "SkillBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SOULBASTION_API USkillBase : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

	//FTickableGameObject requirements
		
	virtual void Tick(float DeltaTime) override;
	
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	UFUNCTION(BlueprintCallable, Category = "Ability System")
	void RequestTick();
	UFUNCTION(BlueprintCallable, Category = "Ability System")
	void ReleaseTick();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Skill")
	bool CanActivate(FGameplayTag Tag);
	virtual bool CanActivate_Implementation(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="Skill | Timing")
	void BeginRecharge(int32 ChargeToConsume = 1);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Skill | Event")
	void OnSkillStateChanged(FGameplayTag EventSkillTag, ESkillState NewState, float NewDuration);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Skill | Event")
	void OnAnimMontageEvent(FAnimMontageData EventData);
	

	UFUNCTION(BlueprintCallable, Category= "Skill | Timing")
	void Recharge(float DeltaTime);
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category= "Skill | Event")
	void OnActivation(FGameplayTag Tag, EActivationInput Input, float ElapsedTime);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category= "Skill | Event")
	void OnOwnerDeath(FOnDeathEvent Payload);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category= "Skill | Event")
	void OnOwnerStatChanged(FStatChangedEvent Payload);

	//Getter Functions

	UFUNCTION(BlueprintPure, Category = "Ability System")
	UAbilitySystem* GetOwningAbility() const {return AbilitySystemRef; }

	UFUNCTION(BlueprintPure, Category = "Ability System")
	ACharacter* GetOwningCharacter() const { return OwningCharacterRef; }
	
	UFUNCTION(BlueprintPure, Category = "Ability System")
	UStatSystem* GetOwnerStats() const {return AbilitySystemRef->GetOwnerStats(); }

	UFUNCTION(BlueprintPure)
	ESkillState GetSkillState() const {return SkillState;}
	
	UFUNCTION(BlueprintPure)
	UAnimInstance* GetOwnerAnimInst() const {return OwningCharacterRef->GetMesh()->GetAnimInstance(); }
	
	//Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SkillTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Priority")
	int32 Priority = 5; // 0 = passive, 5 = normal, 10 = high, 15 = ultimate/dodge

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Priority")
	bool bCanBeInterrupted = true;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Priority")
	bool bCanInterruptSelf = false; // e.g. chain attacks
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill | Data")
	FSkillData SkillData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill | Montage")
	FAbilityMontageParams MontageSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill | FX")
	FFxData VisualEffectsSetting;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill | FX")
	FCameraShakeData CameraShakeSetting;
	
	
	
	//Internal variables
	
	UPROPERTY() 
	int32 TickRequests = 0;
	
	UPROPERTY()
	ESkillState SkillState = ESkillState::Ready;
	
	UPROPERTY()
	float CurrentRechargeTime = 0.f;
	
	UPROPERTY()
	UAbilitySystem* AbilitySystemRef;

	UPROPERTY()
	ACharacter* OwningCharacterRef;
	
};

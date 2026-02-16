// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem.h"
#include "SoulBastion/Data/MyEnums.h"
#include "GameplayTagContainer.h"
#include "UtilityBox.h"
#include "GameFramework/Character.h"
#include "SoulBastion/Data/MyDebug.h"
#include "SkillBase.generated.h"

/**
 * 
 */
class UUtilityBox;
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
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Skill")
	void Activate();
	
	UFUNCTION(BlueprintCallable, Category="Skill | Timing")
	void BeginRecharge(int32 ChargeToConsume = 1);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Skill | Event")
	void OnSkillStateChanged(FGameplayTag EventSkillTag, ESkillState NewState, float NewDuration);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Skill | Event")
	void OnAnimMontageEvent(FAnimMontageData EventData);
	

	UFUNCTION(BlueprintCallable, Category= "Skill | Timing")                 
	void Recharge(float DeltaTime);
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category= "Skill | Event")
	void OnActivation(FGameplayTag Tag, FMyPlayerInput PlayerInput);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category= "Skill | Event")
	void OnOwnerDeath(FOnDeathEvent Payload);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category= "Skill | Event")
	void OnOwnerStatChanged(FStatChangedEvent Payload);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category= "Skill | Event")
	void OnOwnerHitResponse(FHitInfo Payload);

	//Getter Functions

	UFUNCTION(BlueprintPure, Category = "Ability System")
	UAbilitySystem* GetOwningAbility() const {return AbilitySystemRef; }

	UFUNCTION(BlueprintPure, Category = "Ability System")
	ACharacter* GetOwningCharacter() const { return OwningCharacterRef; }
	
	UFUNCTION(BlueprintPure, Category = "Ability System")
	UStatSystem* GetOwnerStats() const {return AbilitySystemRef->GetOwnerStats(); }

	UFUNCTION(BlueprintPure)
	ESkillState GetSkillState(float &StateDuration) const;
	
	UFUNCTION(BlueprintPure)
	UAnimInstance* GetOwnerAnimInst() const {return OwningCharacterRef->GetMesh()->GetAnimInstance(); }
	
	UFUNCTION(BlueprintPure)
	UUtilityBox* GetUtilityBox() const;
	
	
	
	//Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SkillTag;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Activation Tags")
	FGameplayTag ActivationGrantedOwnerTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta Data")
	FSkillData SkillData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage Setting")
	FAbilityMontageParams MontageSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FFxData VisualEffectsSetting;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	FCameraShakeData CameraShakeSetting;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Activation Priority Rules ")
	int32 Priority = 5; // 0 = passive, 5 = normal, 10 = high, 15 = ultimate/dodge
	
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Activation Priority Rules")
	EInterruptibleState LowerPriorityInterruptWindow = EInterruptibleState::None;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Activation Priority Rules")
	EInterruptibleState EqualPriorityInterruptWindow = EInterruptibleState::None;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Activation Priority Rules")
	EInterruptibleState SelfInterruptWindow = EInterruptibleState::None;
	
	
	//Internal variables
	
	UPROPERTY() 
	int32 TickRequests = 0;
	
	UPROPERTY()
	ESkillState SkillState = ESkillState::Ready;
	
	UPROPERTY()
	float CurrentStateDuration = 0.f;
	
	UPROPERTY()
	float CurrentRechargeTime = 0.f;
	
	UPROPERTY()
	UAbilitySystem* AbilitySystemRef;

	UPROPERTY()
	ACharacter* OwningCharacterRef;
	
};

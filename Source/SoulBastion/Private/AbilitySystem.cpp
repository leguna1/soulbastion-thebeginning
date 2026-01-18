#include "AbilitySystem.h"

#include "GameFramework/Character.h"
#include "PlayerCamera.h"
#include "StatSystem.h"
#include "MotionWarpingComponent.h"
#include "SkillBase.h"



UAbilitySystem::UAbilitySystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAbilitySystem::BeginPlay()
{
	Super::BeginPlay();

	InitializeOwner();

	if (!OwnerCharacter) return;

	SetupReferences();
	SetupMotionWarping();
	SetupSkills();
	
	SetupEventBindings();
}
void UAbilitySystem::InitializeOwner()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	OwnerCharacter = Cast<ACharacter>(Owner);
	
}
void UAbilitySystem::SetupReferences()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	StatSystemRef = Owner->FindComponentByClass<UStatSystem>();
	PlayerCameraRef = Owner->FindComponentByClass<UPlayerCamera>();
}
void UAbilitySystem::SetupEventBindings()
{
	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
		return;
	
	if (PlayerCameraRef)
	{
		PlayerCameraRef->OnTargetChanged.AddDynamic(this, &UAbilitySystem::OnTargetChangedHandler);
		UE_LOG(LogTemp, Log, TEXT("Subscribed to PlayerCamera events"));
	}
	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageStarted.AddDynamic(this, &UAbilitySystem::OnMontageStartHandler);
		AnimInstance->OnMontageEnded.AddDynamic(this, &UAbilitySystem::OnMontageEndHandler);
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UAbilitySystem::HandleNotifyBegin);
	}
	
}
void UAbilitySystem::SetupMotionWarping()
{
	if (!OwnerCharacter) return;

	MotionWarpingComponentRef = OwnerCharacter->FindComponentByClass<UMotionWarpingComponent>();
	
	MotionWarpingComponentRef = NewObject<UMotionWarpingComponent>
	(
		OwnerCharacter,
		UMotionWarpingComponent::StaticClass(),
		TEXT("MotionWarpingComponent")
	);

	if (MotionWarpingComponentRef)
	{
		MotionWarpingComponentRef->RegisterComponent();
		UE_LOG(LogTemp, Log, TEXT("Created MotionWarpingComponent dynamically"));
	}
}
void UAbilitySystem::SetupSkills()
{
	for (TSubclassOf<USkillBase> SkillClass : SkillClasses)
	{
		if (!* SkillClass) continue;

		if (USkillBase* NewSkill = NewObject<USkillBase>(this, SkillClass))
		{
			NewSkill->AbilitySystemRef = this;
			NewSkill->OwningCharacterRef = OwnerCharacter;
			this->OnActiveSkillStateChanged.AddDynamic(NewSkill, &USkillBase::OnSkillStateChanged);
			this->OnMontageEvent.AddDynamic(NewSkill, &USkillBase::OnAnimMontageEvent);
			this->OnActivationInput.AddDynamic(NewSkill, &USkillBase::OnActivation);
			StatSystemRef->OnDeath.AddDynamic(NewSkill, &USkillBase::OnOwnerDeath);
			StatSystemRef->OnStatChanged.AddDynamic(NewSkill, &USkillBase::OnOwnerStatChanged);
			this->OnHitResponse.AddDynamic(NewSkill, &USkillBase::OnOwnerHitResponse);
			
			SkillInstances.Add(NewSkill);
			NewSkill->SkillData.CurrentCharge = NewSkill->SkillData.MaxCharge;
			
		}
	}
}

//Main Functions
void UAbilitySystem::TryActivateAbility(const FGameplayTag SkillTag, const FMyPlayerInput PlayerInput)
{
	
	UE_LOG(LogTemp, Warning, TEXT("[Ability] TryActivateAbility: %s"), *SkillTag.ToString());

    USkillBase* RequestedSkill = GetSkillByTag(SkillTag);
    if (!RequestedSkill)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ability] Skill NOT FOUND"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Ability] RequestedSkill valid"));

    // No active skill
    if (!IsValid(ActiveSkill))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ability] No ActiveSkill"));

        if (RequestedSkill->CanActivate_Implementation(SkillTag))
        {
            UE_LOG(LogTemp, Warning, TEXT("[Ability] Activating (no active skill)"));
            OnActivationInput.Broadcast(SkillTag, PlayerInput);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[Ability] CanActivate failed (no active skill)"));
        }
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[Ability] ActiveSkill: %s | State=%d"),
        *ActiveSkill->SkillTag.ToString(),
        (int32)ActiveSkill->SkillState);

    // Same skill
    if (ActiveSkill->SkillTag.MatchesTagExact(SkillTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ability] Same skill requested"));

        const bool bInterruptible =
            IsStateInterruptible(ActiveSkill->SkillState, ActiveSkill->SelfInterruptWindow);

        UE_LOG(LogTemp, Warning,
            TEXT("[Ability] SelfInterruptible=%d CanActivate=%d BufferOpen=%d"),
            bInterruptible,
            RequestedSkill->CanActivate_Implementation(SkillTag),
            bBufferWindowOpen);

        if (bInterruptible && RequestedSkill->CanActivate_Implementation(SkillTag))
        {
            UE_LOG(LogTemp, Warning, TEXT("[Ability] Self-interrupt → Reactivate"));
            OnActivationInput.Broadcast(SkillTag, PlayerInput);
        }
        else if (bBufferWindowOpen)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Ability] Buffering SAME skill"));
            BufferedInputs.Add({ SkillTag, PlayerInput.Input, PlayerInput.InputAction, PlayerInput.InElapsedTime });
        }
        return;
    }

    // Different skill
    const bool bIsHigherPriorityRequest =
        RequestedSkill->Priority > ActiveSkill->Priority;

    UE_LOG(LogTemp, Warning,
        TEXT("[Ability] Different skill | HigherPriority=%d"),
        bIsHigherPriorityRequest);

    if (!bIsHigherPriorityRequest)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ability] Ignored (lower priority)"));
        return;
    }

    const bool bInterruptible = IsStateInterruptible(ActiveSkill->SkillState, RequestedSkill->LowerPriorityInterruptWindow);

    UE_LOG(LogTemp, Warning,
        TEXT("[Ability] Interruptible=%d CanActivate=%d BufferOpen=%d"),
        bInterruptible,
        RequestedSkill->CanActivate_Implementation(SkillTag),bBufferWindowOpen);

    if (RequestedSkill->CanActivate_Implementation(SkillTag) && bInterruptible)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ability] Interrupting → Activate"));
        ResetActiveSkill();
        OnActivationInput.Broadcast(SkillTag, PlayerInput);
    }
    else if (bBufferWindowOpen)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Ability] Buffering DIFFERENT skill"));
        BufferedInputs.Add({ SkillTag, PlayerInput.Input, PlayerInput.InputAction, PlayerInput.InElapsedTime });
    }
	
}
USkillBase* UAbilitySystem::GetSkillByTag(const FGameplayTag& Tag) const
{
	for (USkillBase* Skill : SkillInstances)
	{
		if (Skill && Skill->SkillTag == Tag)
		{
			return Skill;
		}
	}

	return nullptr;
}
void UAbilitySystem::ResetActiveSkill()
{
	if (!IsValid(ActiveSkill))
	{
		return;
	}
	
	if (const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
	{
		if (UAnimInstance* Anim = Mesh->GetAnimInstance())
		{
			if (ActiveSkill->MontageSettings.MontageToPlay && Anim->Montage_IsPlaying(ActiveSkill->MontageSettings.MontageToPlay))
			{
				const float BlendOutTime = ActiveSkill->MontageSettings.MontageToPlay->BlendOut.GetBlendTime();

				Anim->Montage_Stop(BlendOutTime, ActiveSkill->MontageSettings.MontageToPlay);
			}
		}
	}
	ActiveSkill->SkillState = ESkillState::Ready;
	ActiveActionTag = FGameplayTag::EmptyTag;
	ActiveSkill = nullptr;
}
//Sub-Functions
void UAbilitySystem::SetActiveSkillState(const ESkillState NewState, const float StateDuration) const
{
	if (!ActiveSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetActiveSkillState: No active skill to set!"));
		return;
	}

	ActiveSkill->SkillState = NewState;
	ActiveSkill->CurrentStateDuration = StateDuration;
	OnActiveSkillStateChanged.Broadcast(ActiveSkill->SkillTag, NewState, StateDuration);
}

FSkillData UAbilitySystem::GetSkillData(const FGameplayTag SkillTag) const
{
	for (USkillBase* Skill : SkillInstances)
	{
		if (Skill && Skill->SkillTag == SkillTag)
		{
			return Skill->SkillData;
		}
	}

	return FSkillData(); 
}
void UAbilitySystem::GetCurrentlyActiveSkill(bool &ReturnValue, USkillBase* &Instance) const
{
	
	if (IsValid(ActiveSkill))
	{
		ReturnValue = true;
		Instance = ActiveSkill;
	}
	else
	{
		ReturnValue = false;
		Instance = nullptr;
	}
}
//Event handlers
void UAbilitySystem::OnTargetChangedHandler(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
}

void UAbilitySystem::PlayAbilityMontage(const FAbilityMontageParams& Params, const bool bUseMotionWarp)
{
	if (!OwnerCharacter || !Params.MontageToPlay)
	{
		FAnimMontageData Data;
		Data.State = EAnimMontageState::Interrupted;
		OnMontageEvent.Broadcast(Data);
		return;
	}
	
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	CurrentCurveTime = 0.f;

	if (!AnimInstance) return;

	if (bUseMotionWarp && MotionWarpingComponentRef)
	{
		ApplyMotionWarp(bUseMotionWarp, Params);
		CurrentWarpName = Params.WarpName; // store warp target name for cleanup
	}
	else
	{
		CurrentWarpName = NAME_None;
	}
	
	// Play the montage with initial rate
	float NormalizedTime = 0.f;
	float CurveRate = Params.PlayRateCurve ? Params.PlayRateCurve->GetFloatValue(NormalizedTime) : 1.f;
	float FinalRate = Params.PlayRate * CurveRate * (1 + Params.PlayRateMultiplayer);
	
	AnimInstance->Montage_Play(Params.MontageToPlay, FinalRate);

	if (Params.StartSection != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(Params.StartSection, Params.MontageToPlay);
	}
	
	// Broadcast Started event
	FAnimMontageData StartedData;
	StartedData.State = EAnimMontageState::Started;
	StartedData.NormalizedTime = 0.f;
	OnMontageEvent.Broadcast(StartedData);

	// Start timer to simulate Tick at ~60 FPS
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &UAbilitySystem::UpdateMontageTick, Params.MontageToPlay, Params, Params.PlayRateMultiplayer);
	GetWorld()->GetTimerManager().SetTimer(MontageUpdateTimerHandle, TimerDel, 0.0167, true);
}
void UAbilitySystem::ApplyMotionWarp(bool bUseWarp, const FAbilityMontageParams& Params) const
{
	if (!bUseWarp || !MotionWarpingComponentRef)
		return;

	const FVector InstLoc = OwnerCharacter->GetActorLocation();
	FVector WarpLoc = InstLoc;   // default: no movement
	FRotator WarpRot = OwnerCharacter->GetActorRotation(); // default: keep rotation
	

	switch (Params.TranslationMode)
	{
        
	case EMotionWarpTranslationMode::None:
		{
			// No warp translation at all.
			// WarpLoc = InstLoc already.
			break;
		}
	case EMotionWarpTranslationMode::ToTargetActor:
		{
			if (IsValid(CurrentTarget))
			{
				const FVector TargetLoc = CurrentTarget->GetActorLocation();
				const FVector Dir = (TargetLoc - InstLoc).GetSafeNormal();

				float Distance = FVector::Dist(InstLoc, TargetLoc);
				float Delta = Distance - Params.IdealRange;

				float Clamped = FMath::Clamp(Delta, -Params.MaxWarpDistance, Params.MaxWarpDistance);

				WarpLoc = InstLoc + Dir * Clamped;
			}
			else
			{
				const FVector ForwardDir = OwnerCharacter->GetActorForwardVector();
				WarpLoc = InstLoc + ForwardDir * Params.MaxWarpDistance;
			}
			break;
		}
    
	case EMotionWarpTranslationMode::Forward:
		{
			const FVector ForwardDir = OwnerCharacter->GetActorForwardVector();
			WarpLoc = InstLoc + ForwardDir * Params.MaxWarpDistance;
			break;
		}

        
	case EMotionWarpTranslationMode::Backward:
		{
			const FVector BackDir = -OwnerCharacter->GetActorForwardVector();
			WarpLoc = InstLoc + BackDir * Params.MaxWarpDistance;
			break;
		}
	}

	switch (Params.RotationMode)
	{
	case EMotionWarpRotationMode::None:
		// No rotation override → keep owner's rotation
		break;

	case EMotionWarpRotationMode::ToTargetActor:
		{
			if (IsValid(CurrentTarget))
			{
				FVector TargetLoc = CurrentTarget->GetActorLocation();
				FRotator LookRot = (TargetLoc - InstLoc).Rotation();

				// Only yaw rotation (keep natural movement)
				LookRot.Pitch = 0.f;
				LookRot.Roll  = 0.f;

				WarpRot = LookRot;
			}
			break;
		}
	}
	
	MotionWarpingComponentRef->AddOrUpdateWarpTargetFromLocationAndRotation(Params.WarpName, WarpLoc, WarpRot);
	
}

void UAbilitySystem::UpdateMontageTick(UAnimMontage* MontageRef, FAbilityMontageParams MontageParams, float PlayRateMultiplier)
{
	if (!ActiveSkill || !OwnerCharacter || !MontageRef)
	{
		GetWorld()->GetTimerManager().ClearTimer(MontageUpdateTimerHandle);
		return;
	}
	  
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(MontageRef))
		return;

	// ---- Detect section change ----
	FName CurrentSection = AnimInstance->Montage_GetCurrentSection(MontageRef);
	
	if (CurrentSection != LastSectionName)
	{
		// Reset curve time for new section!
		CurrentCurveTime = 0.f;
		LastSectionName = CurrentSection;
		// (Optional) Broadcast a "SectionChanged" event if needed
		
	}
	
	// Update play rate dynamically
	UpdateMontagePlayRate(MontageRef, MontageParams, PlayRateMultiplier);

	// Broadcast WhileActive event
	float Pos = AnimInstance->Montage_GetPosition(MontageRef);
	float Len = MontageRef->GetPlayLength();
	float NormalizedTime = (Len > 0.f) ? FMath::Clamp(Pos / Len, 0.f, 1.f) : 0.f;

	FAnimMontageData Data;
	Data.State = EAnimMontageState::WhileActive;
	Data.NormalizedTime = NormalizedTime;
	OnMontageEvent.Broadcast(Data);
}

void UAbilitySystem::UpdateMontagePlayRate(const UAnimMontage* MontageRef, const FAbilityMontageParams& MontageParams, float PlayRateMultiplier) const
{
	if (!MontageRef || !OwnerCharacter)
		return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	float Pos = AnimInstance->Montage_GetPosition(MontageRef);
	float Len = MontageRef->GetPlayLength();
	float NormalizedTime = (Len > 0.f) ? FMath::Clamp(Pos / Len, 0.f, 1.f) : 0.f;

	float CurveRate = MontageParams.PlayRateCurve ? MontageParams.PlayRateCurve->GetFloatValue(NormalizedTime) : 1.f;
	float FinalRate = MontageParams.PlayRate * CurveRate * (1 + PlayRateMultiplier);

	AnimInstance->Montage_SetPlayRate(MontageRef, FinalRate);
}
void UAbilitySystem::OnMontageStartHandler(UAnimMontage* Montage)
{
	if (!Montage) return;

	// If there is an active skill already, it becomes the old skill
	
	if (ActiveSkill && ActiveSkill->MontageSettings.MontageToPlay != Montage)
	{
		OldActiveSkill = ActiveSkill;
	} 

	// Find the skill instance that matches this montage
	for (USkillBase* Skill : SkillInstances)
	{
		if (!Skill || Skill->MontageSettings.MontageToPlay != Montage)
			continue;

		// Set new active skill
		ActiveSkill = Skill;

		// Update skill state
		SetActiveSkillState(ESkillState::Active, 0.0f);
		
		ActiveActionTag = ActiveSkill->ActivationGrantedOwnerTag; 

		// Notify listeners
		OnActiveSkillStateChanged.Broadcast(Skill->SkillTag, Skill->SkillState, 0.f);

		return; // done
	}
}
void UAbilitySystem::OnMontageEndHandler(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	// First, handle old skill if its montage matches
	
	if (OldActiveSkill && OldActiveSkill->MontageSettings.MontageToPlay == Montage)
	{
		// Broadcast Completed or Interrupted
		FAnimMontageData Data;
		Data.State = bInterrupted ? EAnimMontageState::Interrupted : EAnimMontageState::Completed;
		Data.NormalizedTime = 1.f;
		OnMontageEvent.Broadcast(Data);
		
		// Reset old skill state
		OldActiveSkill->SkillState = ESkillState::Ready;
		
		OnActiveSkillStateChanged.Broadcast(OldActiveSkill->SkillTag, ESkillState::Ready, 0.f);

		OldActiveSkill = nullptr;
	}
	
	// Then, handle active skill if its montage ends
	if (ActiveSkill && ActiveSkill->MontageSettings.MontageToPlay == Montage)
	{
		FAnimMontageData Data;
		Data.State = bInterrupted ? EAnimMontageState::Interrupted : EAnimMontageState::Completed;
		Data.NormalizedTime = 1.f;
		OnMontageEvent.Broadcast(Data);

		ActiveSkill->SkillState = ESkillState::Ready;
		OnActiveSkillStateChanged.Broadcast(ActiveSkill->SkillTag, ESkillState::Ready, 0.f);
		ActiveActionTag = FGameplayTag::EmptyTag;

		ActiveSkill = nullptr;
	}
	
	// Stop any ongoing montage update timers
	GetWorld()->GetTimerManager().ClearTimer(MontageUpdateTimerHandle);

	// Remove motion warp target
	if (MotionWarpingComponentRef && CurrentWarpName != NAME_None)
	{
		MotionWarpingComponentRef->RemoveWarpTarget(CurrentWarpName);
		CurrentWarpName = NAME_None;
	}
}

bool UAbilitySystem::IsStateInterruptible(const ESkillState CurrentState, const EInterruptibleState InterruptRule)
{
	if (InterruptRule == EInterruptibleState::Any)
	{
		return true;
	}
	
	switch (InterruptRule)
	{
	case EInterruptibleState::None:
		return false;
		
	case EInterruptibleState::Windup:
		return CurrentState == ESkillState::Windup;
		
	case EInterruptibleState::Active:
		return CurrentState == ESkillState::Active;
		
	case EInterruptibleState::Recovery:
		return CurrentState == ESkillState::Recovery;
		
	case EInterruptibleState::WindupOrActive:
		return CurrentState == ESkillState::Windup || CurrentState == ESkillState::Active;
		
	case EInterruptibleState::WindupOrRecovery:
		return CurrentState == ESkillState::Windup || CurrentState == ESkillState::Recovery;
		
	case EInterruptibleState::ActiveOrRecovery:
		return CurrentState == ESkillState::Active || CurrentState == ESkillState::Recovery;
		
		
	default:
		return false;
	}
	
}

void UAbilitySystem::HandleNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	FAnimMontageData Data;
	Data.State = EAnimMontageState::NotifyBegin;
	Data.NotifyName = NotifyName;

	OnMontageEvent.Broadcast(Data);
}

EHitResult UAbilitySystem::HitResponse(const FHitInfo InHitInfo) const
{
	const FGameplayTag HealthTag = FGameplayTag::RequestGameplayTag("Stat.Health");
	const FGameplayTag ArmorTag  = FGameplayTag::RequestGameplayTag("Stat.Armor");
	
	if (ActiveActionTag == FGameplayTag::EmptyTag)
	{
		StatSystemRef-> ModifyStat(InHitInfo.SourceActor, HealthTag, EStatValueType::BaseValue, -InHitInfo.DamageAmount);
		OnHitResponse.Broadcast(InHitInfo);
		return EHitResult::Damaged;
	}
	if (ActiveActionTag == FGameplayTag::RequestGameplayTag("Combat.Attacking"))
	{
		StatSystemRef-> ModifyStat(InHitInfo.SourceActor, HealthTag, EStatValueType::BaseValue, -InHitInfo.DamageAmount);
		OnHitResponse.Broadcast(InHitInfo);
		return EHitResult::Clashed;
	}
	if (ActiveActionTag ==  FGameplayTag::RequestGameplayTag("Combat.Blocking"))
	{
		const float DamageAfterArmor = FMath::Max(0.f, InHitInfo.DamageAmount - StatSystemRef->GetStatValue(ArmorTag, EStatValueType::Value));
		StatSystemRef-> ModifyStat(InHitInfo.SourceActor, HealthTag, EStatValueType::BaseValue, -DamageAfterArmor);
		OnHitResponse.Broadcast(InHitInfo);
		return EHitResult::Blocked;
	}
	if (ActiveActionTag == FGameplayTag::RequestGameplayTag("Combat.Dodging"))
	{
		return EHitResult::Dodged;
	}
	
	return EHitResult::None;
}

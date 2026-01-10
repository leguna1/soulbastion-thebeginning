// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillActiveState.h"

void USkillActiveState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	// Ignore editor preview and animation tools
	if (!MeshComp || !MeshComp->GetWorld() || !MeshComp->GetWorld()->IsGameWorld())
	{
		return;
	}
	CachedOwner = MeshComp->GetOwner();
	if (!CachedOwner) return;

	CachedAbilitySystem = CachedOwner->FindComponentByClass<UAbilitySystem>();
	
	if (!CachedAbilitySystem) return;
	
	CachedAbilitySystem->SetActiveSkillState(ESkillState::Active, TotalDuration);
	
}

void USkillActiveState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!CachedOwner) return;
	if (!CachedAbilitySystem) return;
	
	CachedAbilitySystem->SetActiveSkillState(ESkillState::Recovery, 0.5f);
	
	// Clear cached references
	CachedOwner = nullptr;
	CachedAbilitySystem = nullptr;
}

#include "BufferWindow.h"
#include "AbilitySystem.h"
#include "SkillBase.h"

void UBufferWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	// Ignore editor preview and animation tools
	if (!MeshComp || !MeshComp->GetWorld() || !MeshComp->GetWorld()->IsGameWorld())
	{
		return;
	}
	
	CachedOwner = MeshComp->GetOwner();
	if (!CachedOwner)
		return;

	CachedAbilitySystem = CachedOwner->FindComponentByClass<UAbilitySystem>();
	if (CachedAbilitySystem)
	{
		CachedAbilitySystem->bBufferWindowOpen = true;
		
		CachedAbilitySystem->SetActiveSkillState(ESkillState::Active, TotalDuration);
		
		UE_LOG(LogTemp, Log, TEXT("Buffer window opened."));
	}
}

void UBufferWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!CachedAbilitySystem)
		return;
	
	UAbilitySystem* AS = CachedAbilitySystem; 
	AS->bBufferWindowOpen = false;
	
	
	if (AS->ActiveSkill)
	{
		AS->SetActiveSkillState(ESkillState::Recovery, 0.f);
	}
	

	if (AS->BufferedInputs.Num() > 0)
	{
		const FBufferedInput& Last  = AS->BufferedInputs.Last();
		
		UE_LOG(LogTemp,  Log,
			TEXT("Flushing buffered input: %s, input=%d"),
			*Last.SkillTag.ToString(),
			(int)Last.Input);
		
		AS->OnActivationInput.Broadcast(
			Last.SkillTag,
			Last.Input,
			Last.ElapsedTime);
		
		AS->BufferedInputs.Empty();
	}
	
	
	// Clear cached references
	CachedOwner = nullptr;
	CachedAbilitySystem = nullptr;

	UE_LOG(LogTemp, Log, TEXT("Buffer window closed."));
}

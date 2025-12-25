#include "SkillStateNotify.h"
#include "AbilitySystem.h"

void USkillStateNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
							   const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetWorld() || !MeshComp->GetWorld()->IsGameWorld())
		return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
		return;

	if (!SkillTag.IsValid())
		return;

	// ALWAYS check first
	UAbilitySystem* AbilitySystem = Owner->FindComponentByClass<UAbilitySystem>();
	if (!IsValid(AbilitySystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillStateNotify: No AbilitySystem found on %s"), *Owner->GetName());
		return;
	}

	// Check ActiveSkill BEFORE reading from it
	if (!AbilitySystem->ActiveSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillStateNotify: ActiveSkill is null on %s"), *Owner->GetName());
		return;
	}

	FGameplayTag CurrentlyActiveTag = AbilitySystem->ActiveSkill->SkillTag;
	if (!CurrentlyActiveTag.IsValid())
		return;

	// MATCHES THE SKILL?
	if (CurrentlyActiveTag == SkillTag)
	{
		AbilitySystem->SetActiveSkillState(NewState, StateDuration);

		UE_LOG(LogTemp, Log, TEXT("SkillStateNotify: Setting %s to %s"),
			*SkillTag.ToString(),
			*StaticEnum<ESkillState>()->GetNameStringByValue((int64)NewState));
	}
}
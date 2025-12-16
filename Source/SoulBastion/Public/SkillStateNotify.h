#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "SkillBase.h"
#include "SkillStateNotify.generated.h"

UCLASS()
class SOULBASTION_API USkillStateNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
	FGameplayTag SkillTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
	ESkillState NewState = ESkillState::Ready;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
						const FAnimNotifyEventReference& EventReference) override;
};
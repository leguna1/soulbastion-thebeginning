#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "BufferWindow.generated.h"

class UAbilitySystem;

UCLASS()
class SOULBASTION_API UBufferWindow : public UAnimNotifyState
{
	GENERATED_BODY()

private:
	UPROPERTY()
	AActor* CachedOwner = nullptr;

	UPROPERTY()
	UAbilitySystem* CachedAbilitySystem = nullptr;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
							 float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
						   const FAnimNotifyEventReference& EventReference) override;
};

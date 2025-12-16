#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SB_AIController.generated.h"

class UAISenseConfig_Sight;
class UStatSystem;
class UAbilitySystem;

UCLASS()
class SOULBASTION_API ASB_AIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ASB_AIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAIPerceptionComponent* PerceptionComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAISenseConfig_Sight* SightConfig;

	//Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	ACharacter* AICharacter = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStatSystem* StatsComp = nullptr;
	
};

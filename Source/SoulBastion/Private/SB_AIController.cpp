#include "SB_AIController.h"
#include "StatSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Character.h"


ASB_AIController::ASB_AIController()
{
	bAttachToPawn = true;
	PrimaryActorTick.bCanEverTick = true;

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SightConfig    = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
}

void ASB_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AICharacter = Cast<ACharacter>(InPawn);
    
	// Setup stats etc...
	if (AICharacter)
	{
		StatsComp = AICharacter->FindComponentByClass<UStatSystem>();
	}

	// ---- CONFIGURE SIGHT ----
	SightConfig->SightRadius = 750.f;
	SightConfig->LoseSightRadius = 1000.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->SetMaxAge(3.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
	
}

void ASB_AIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}
void ASB_AIController::BeginPlay()
{
	Super::BeginPlay();
}

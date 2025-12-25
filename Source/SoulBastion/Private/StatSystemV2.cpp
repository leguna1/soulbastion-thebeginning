#include "StatSystemV2.h"

// Sets default values for this component's properties
UStatSystemV2::UStatSystemV2()
{
	
	PrimaryComponentTick.bCanEverTick = true;

}

void UStatSystemV2::BeginPlay()
{
	Super::BeginPlay();
	
	//Construct the stats and
	
}

void UStatSystemV2::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UStatSystemV2::GetStatValues(FGameplayTag StatTag, float& OutValue, float& OutMaxValue)
{
	
}

void UStatSystemV2::SetStatValue(FGameplayTag StatTag, float NewValue, bool bMaxValue)
{
	
}

void UStatSystemV2::ModifyStatValue(FGameplayTag StatTag, float NewValue, bool bMaxValue)
{
	
}

void UStatSystemV2::RevertLastModification(FGameplayTag StatTag)
{
	
}



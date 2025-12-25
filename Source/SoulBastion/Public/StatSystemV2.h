#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulBastion/Data/MyStructs.h"
#include "StatBase.h"
#include "StatSystemV2.generated.h"



//2 event dispatchers are needed, one for stat value change and one for state change, that blueprint children of stat base can listen to.
class UStatBase;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULBASTION_API UStatSystemV2 : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UStatSystemV2();

protected:
	
	virtual void BeginPlay() override;

public:	
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<UStatBase*> Stats;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat System V2")
	FGameplayTagContainer StateTags;
	
	
	UFUNCTION(BlueprintCallable, Category = "Stat System V2 | Getter")
	TArray<UStatBase*> GetAllStats() const {return Stats; }
	
	UFUNCTION(BlueprintCallable, Category = "Stat System V2 | Getter")
	void GetStatValues(FGameplayTag StatTag, float& OutValue, float& OutMaxValue);
	
	UFUNCTION(BlueprintCallable, Category = "Stat System V2 | Setter")
	void SetStatValue(FGameplayTag StatTag, float NewValue, bool bMaxValue);
	
	UFUNCTION(BlueprintCallable, Category = "Stat System V2 | Modifier")
	void ModifyStatValue(FGameplayTag StatTag, float NewValue, bool bMaxValue);
	
	UFUNCTION(BlueprintCallable, Category = "Stat System V2 | Modifier")
	void RevertLastModification(FGameplayTag StatTag);
	
	
};

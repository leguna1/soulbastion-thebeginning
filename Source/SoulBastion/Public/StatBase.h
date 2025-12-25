// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulBastion/Data/MyStructs.h"
#include "StatBase.generated.h"

/**
 * 
 */
class UStatSystemV2;

UCLASS(Blueprintable, BlueprintType)
class SOULBASTION_API UStatBase : public UObject
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FStatData StatData;
	
};

// MyDebug.h
#pragma once

#include "CoreMinimal.h"

#define ENABLE_MY_DEBUG 0 // Set to 0 to disable all debug messages

#if ENABLE_MY_DEBUG
	#define DEBUG_LOG(Message, ...) \
	{ \
	UE_LOG(LogTemp, Warning, TEXT(Message), ##__VA_ARGS__); \
	if (GEngine) \
	{ \
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT(Message), ##__VA_ARGS__)); \
	} \
	}
#else
	#define DEBUG_LOG(Message, ...) 
#endif

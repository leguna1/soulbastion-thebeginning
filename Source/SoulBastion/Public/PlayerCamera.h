#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "PlayerCamera.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, AActor*, NewTarget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOULBASTION_API UPlayerCamera : public UCameraComponent
{
    GENERATED_BODY()

public:
    UPlayerCamera();
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle TargetUpdateTimer;

    void UpdateAutoTarget();
    AActor* FindBestTarget() const;
    bool IsActorAlive(AActor* Actor) const;

public:
    // -------------------
    // Targeting
    // -------------------
    UPROPERTY(BlueprintReadOnly, Category="Targeting")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
    float TargetRadius = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
    float FOVHalfAngle = 45.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
    TSubclassOf<AActor> TargetActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting|Debug")
    bool bDebug = false;

    // -------------------
    // Event when target changes
    // -------------------
    UPROPERTY(BlueprintAssignable, Category="Targeting")
    FOnTargetChanged OnTargetChanged;
};

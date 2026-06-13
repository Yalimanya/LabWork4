#pragma once

#include "CoreMinimal.h"
#include "NetBaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "NetAvatar.generated.h"

UCLASS()
class ANetAvatar : public ANetBaseCharacter
{
    GENERATED_BODY()

public:
    ANetAvatar();

    UPROPERTY(EditAnywhere)
    UCameraComponent* Camera;

    UPROPERTY(EditAnywhere)
    USpringArmComponent* SpringArm;

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementScale;

private:
    void MoveForward(float Amount);
    void MoveRight(float Amount);
};
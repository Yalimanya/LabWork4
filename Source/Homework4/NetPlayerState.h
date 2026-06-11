#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NetGameInstance.h" 
#include "NetPlayerState.generated.h"

UCLASS()
class HOMEWORK4_API ANetPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Data")
    FSPlayerInfo PlayerInfo;

    
};
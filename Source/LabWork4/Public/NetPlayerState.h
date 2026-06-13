#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NetGameInstance.h"
#include "NetPlayerState.generated.h"

UCLASS()
class LABWORK4_API ANetPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    // Holds the player's nickname and visual customization data
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerInfo)
    FSPlayerInfo Data;

    // Index to identify the player
    UPROPERTY(BlueprintReadOnly, Replicated)
    int PlayerIndex;

    // Blue or Red team assignment
    UPROPERTY(BlueprintReadOnly, Replicated)
    EPlayerTeam TeamID;

    // Result of the match for this specific player
    UPROPERTY(BlueprintReadOnly, Replicated)
    EGameResults Result;

private:
    // Triggers when the client receives the player info
    UFUNCTION()
    void OnRep_PlayerInfo();
};
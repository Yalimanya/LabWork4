#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NetGameState.generated.h"

class ANetPlayerState;

UCLASS()
class LABWORK4_API ANetGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ANetGameState();

    // Replicated to all clients to trigger match restart
    UFUNCTION(NetMulticast, Reliable)
    void TriggerRestart();

    // Holds the index of the winning player
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Winner)
    int WinningPlayer;

    UFUNCTION()
    void OnRep_Winner();

    // Blueprint events for UI and animations
    UFUNCTION(BlueprintImplementableEvent)
    void OnVictory();

    UFUNCTION(BlueprintImplementableEvent)
    void OnRestart();

    // Helper to find a specific player's state
    UFUNCTION(BlueprintCallable)
    ANetPlayerState* GetPlayerStateByIndex(int PlayerIndex);
};
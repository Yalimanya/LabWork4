#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetGameMode.generated.h"

UCLASS()
class ANetGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    // Overriding BeginPlay to start the timer
    virtual void BeginPlay() override;

    // Function that triggers when the 30-second timer ends
    UFUNCTION()
    void OnBlueTeamTimeoutWin();

    // Function to handle the end of the game and swap mechanics
    UFUNCTION(BlueprintCallable, Category = "Game Rules")
    void EndGame(bool bBlueTeamWon);

protected:
    // Timer handle to manage the 30-second countdown
    FTimerHandle BlueTeamWinTimer;
};
#include "NetGameMode.h"
#include "NetGameState.h"
#include "NetPlayerState.h"
#include "NetGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

void ANetGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Start a 30-second timer that calls OnBlueTeamTimeoutWin when it finishes.
    // The 'false' parameter means it will not loop.
    GetWorldTimerManager().SetTimer(BlueTeamWinTimer, this, &ANetGameMode::OnBlueTeamTimeoutWin, 30.0f, false);

    // Debug message to verify timer has started
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Timer Started: Survive for 30 seconds!"));
}

void ANetGameMode::OnBlueTeamTimeoutWin()
{
    // This triggers if the 30 seconds pass without the Red Team catching the Blue Team player
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Blue Team survived 30 seconds and won!"));

    // End the game with Blue Team as the winner
    EndGame(true);
}

void ANetGameMode::EndGame(bool bBlueTeamWon)
{
    // Clear the timer just in case EndGame was called early (e.g., Red Team caught Blue Team)
    GetWorldTimerManager().ClearTimer(BlueTeamWinTimer);

    if (bBlueTeamWon)
    {
        ANetPlayerState* BluePlayer = nullptr;
        TArray<ANetPlayerState*> RedPlayers;

        // Iterate through all players in the GameState to find the Blue player and collect all Red players
        if (GameState)
        {
            for (APlayerState* PState : GameState->PlayerArray)
            {
                ANetPlayerState* NetPState = Cast<ANetPlayerState>(PState);
                if (NetPState)
                {
                    if (NetPState->PlayerInfo.TeamID == EPlayerTeam::TEAM_Blue)
                    {
                        BluePlayer = NetPState;
                    }
                    else if (NetPState->PlayerInfo.TeamID == EPlayerTeam::TEAM_Red)
                    {
                        RedPlayers.Add(NetPState);
                    }
                }
            }
        }

        // ASSIGNMENT LOGIC: Swap places if Blue team won
        // LOGIC EXPLANATION: We pick a random Red team member to swap with the Blue team member.
        // This ensures every Red player has an equal and fair chance of becoming the "runner" (Blue) in the next round.
        if (BluePlayer != nullptr && RedPlayers.Num() > 0)
        {
            // Pick a random index from the RedPlayers array
            int32 RandomIndex = FMath::RandRange(0, RedPlayers.Num() - 1);
            ANetPlayerState* ChosenRedPlayer = RedPlayers[RandomIndex];

            // Perform the swap
            BluePlayer->PlayerInfo.TeamID = EPlayerTeam::TEAM_Red;
            ChosenRedPlayer->PlayerInfo.TeamID = EPlayerTeam::TEAM_Blue;

            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Teams Swapped! Restarting round soon..."));
        }
    }
    else
    {
        // Red team won logic (e.g., they caught the Blue player before 30 seconds)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Red Team caught the Blue Player and won!"));
    }

    // Call the GameState function to show the results screen widget (from Week 10 PDF)
    // if (ANetGameState* NetGameState = GetGameState<ANetGameState>())
    // {
    //     NetGameState->ShowResultsScreen();
    // }
}
#include "NetGameState.h"
#include "NetPlayerState.h"
#include "NetBaseCharacter.h"
#include "Net/UnrealNetwork.h"

ANetGameState::ANetGameState() : WinningPlayer(-1)
{
}

void ANetGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANetGameState, WinningPlayer);
}

void ANetGameState::OnRep_Winner()
{
    // Trigger victory logic if a valid winner exists
    if (WinningPlayer >= 0)
    {
        OnVictory();
    }
}

void ANetGameState::TriggerRestart_Implementation()
{
    OnRestart();
}

ANetPlayerState* ANetGameState::GetPlayerStateByIndex(int PlayerIndex)
{
    // Search through all connected players
    for (APlayerState* PS : PlayerArray)
    {
        ANetPlayerState* State = Cast<ANetPlayerState>(PS);
        if (State && State->PlayerIndex == PlayerIndex)
        {
            return State;
        }
    }
    return nullptr;
}
#include "NetPlayerState.h"
#include "NetBaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

void ANetPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Register properties for network replication
    DOREPLIFETIME(ANetPlayerState, Data);
    DOREPLIFETIME(ANetPlayerState, PlayerIndex);
    DOREPLIFETIME(ANetPlayerState, TeamID);
    DOREPLIFETIME(ANetPlayerState, Result);
}

void ANetPlayerState::OnRep_PlayerInfo()
{
    // Notify the character that data has arrived
    ANetBaseCharacter* Char = Cast<ANetBaseCharacter>(GetPawn());
    if (Char)
    {
        Char->PlayerInfoReceived = true;
    }
    else
    {
        // If pawn is not ready yet, check again after 0.25 seconds
        FTimerHandle PlayerInfoUpdateTimer;
        GetWorld()->GetTimerManager().SetTimer(PlayerInfoUpdateTimer, this, &ANetPlayerState::OnRep_PlayerInfo, 0.25f, false);
    }
}
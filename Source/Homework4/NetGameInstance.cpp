#include "NetGameInstance.h"

void UNetGameInstance::Host(FString MapName, FSPlayerInfo Info)
{
    // Print a debug message to the screen indicating hosting started
    GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Red, TEXT("Hosting Game..."));

    // Store the player info locally on the instance
    PlayerInfo = Info;

    // Travel to the specified map as a listen server
    GWorld->ServerTravel(FString::Printf(TEXT("/Game/Maps/%s?listen"), *MapName));
}

void UNetGameInstance::Join(FString Address, FSPlayerInfo Info)
{
    // Print a debug message to the screen indicating joining started
    GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Green, FString::Printf(TEXT("Joining Game at %s..."), *Address));

    // Store the player info locally on the instance
    PlayerInfo = Info;

    // Connect to the target server address
    GWorld->GetFirstPlayerController()->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}
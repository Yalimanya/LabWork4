#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetAvatar.h" // Needed to recognize ANetAvatar
#include "NetGameMode.generated.h"

UCLASS()
class LABWORK4_API ANetGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ANetGameMode();

    virtual void BeginPlay() override;

    // Overrides the default player start selection logic
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    // Triggered when a Red team member catches a Blue team member
    UFUNCTION(BlueprintCallable)
    void AvatarsOverlapped(AActor* AvatarA, AActor* AvatarB);

    // Resets the game and prepares for the next round
    UFUNCTION(BlueprintCallable)
    void EndGame();

private:
    int TotalPlayerCount;
    int TotalGames;
    int PlayerStartIndex;

    // Keeps track of all connected players
    TArray<APlayerController*> AllPlayers;

    // Helper function to find a specific PlayerStart in the map
    AActor* GetPlayerStart(FString Name, int Index);

    // Assigns team and returns the correct PlayerStart
    AActor* AssignTeamAndPlayerStart(AController* Player);

    FTimerHandle MatchTimerHandle;
    void OnMatchTimeOut(); // 30 saniye dolduğunda çalışacak fonksiyon
};
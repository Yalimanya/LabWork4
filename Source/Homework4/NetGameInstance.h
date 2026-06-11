#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NetGameInstance.generated.h"

// Enum to hold team information
UENUM(BlueprintType)
enum class EPlayerTeam : uint8
{
    TEAM_Unknown,
    TEAM_Blue,
    TEAM_Red
};

// Enum to hold game results
UENUM(BlueprintType)
enum class EGameResults : uint8
{
    RESULT_Undefined,
    RESULT_Lost,
    RESULT_Won
};

// Struct to carry player data from the UI
USTRUCT(BlueprintType)
struct FSPlayerInfo
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Nickname;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CustomizationData;

    UPROPERTY(BlueprintReadWrite)
    EPlayerTeam TeamID;
};

UCLASS()
class UNetGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // Functions to be called from UI for hosting/joining sessions
    UFUNCTION(BlueprintCallable)
    void Host(FString MapName, FSPlayerInfo Info);

    UFUNCTION(BlueprintCallable)
    void Join(FString Address, FSPlayerInfo Info);

    // Variable holding the current player's local data
    UPROPERTY(BlueprintReadWrite)
    FSPlayerInfo PlayerInfo;
};
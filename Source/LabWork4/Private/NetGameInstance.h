#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NetGameInstance.generated.h"

// 1. Player Team Enumeration
UENUM(BlueprintType)
enum class EPlayerTeam : uint8
{
    TEAM_Unknown,
    TEAM_Blue,
    TEAM_Red
};

// 2. Game Results Enumeration
UENUM(BlueprintType)
enum class EGameResults : uint8
{
    RESULT_Undefined,
    RESULT_Lost,
    RESULT_Won
};

// 3. Player Info Structure
USTRUCT(BlueprintType)
struct FSPlayerInfo
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Nickname;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CustomizationData;

    UPROPERTY(BlueprintReadOnly)
    EPlayerTeam TeamID;
};

UCLASS()
class LABWORK4_API UNetGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // Host a network game
    UFUNCTION(BlueprintCallable)
    void host(FString MapName, FSPlayerInfo Info);

    // Join a network game
    UFUNCTION(BlueprintCallable)
    void join(FString Address, FSPlayerInfo Info);

    // Stores the local player's information
    UPROPERTY(BlueprintReadWrite)
    FSPlayerInfo PlayerInfo;
};
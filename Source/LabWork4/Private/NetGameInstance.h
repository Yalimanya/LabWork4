#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NetBaseCharacter.h"
#include "NetGameInstance.generated.h"

UCLASS()
class UNetGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void host(FString MapName, FSPlayerInfo Info); 

    UFUNCTION(BlueprintCallable)
    void join(FString Address, FSPlayerInfo Info); 

    
    UPROPERTY(BlueprintReadWrite)
    FSPlayerInfo PlayerInfo;
};
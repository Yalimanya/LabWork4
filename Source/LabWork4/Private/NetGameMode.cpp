#include "NetGameMode.h"
#include "NetBaseCharacter.h"

ANetGameMode::ANetGameMode()
{
    DefaultPawnClass = ANetBaseCharacter::StaticClass();
}
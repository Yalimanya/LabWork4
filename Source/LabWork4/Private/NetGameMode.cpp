#include "NetGameMode.h"
#include "NetBaseCharacter.h"
#include "NetGameState.h"
#include "NetPlayerState.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

ANetGameMode::ANetGameMode()
{
    // Set default classes for the network game
    DefaultPawnClass = ANetBaseCharacter::StaticClass();
    PlayerStateClass = ANetPlayerState::StaticClass();
    GameStateClass = ANetGameState::StaticClass();

    TotalPlayerCount = 0;
    TotalGames = 0;
    PlayerStartIndex = 0;
}

AActor* ANetGameMode::GetPlayerStart(FString Name, int Index)
{
    FName PSName;
    if (Index < 0)
    {
        PSName = FName(*Name);
    }
    else
    {
        // Cycles through Red0, Red1, Red2, Red3
        PSName = FName(*FString::Printf(TEXT("%s%d"), *Name, Index % 4));
    }

    // Search the world for the matching PlayerStart tag
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        if (APlayerStart* PS = Cast<APlayerStart>(*It))
        {
            if (PS->PlayerStartTag == PSName) return *It;
        }
    }
    return nullptr;
}

AActor* ANetGameMode::AssignTeamAndPlayerStart(AController* Player)
{
    AActor* Start = nullptr;
    ANetPlayerState* State = Player->GetPlayerState<ANetPlayerState>();

    if (State)
    {
        if (TotalGames == 0)
        {
            // First game: First player is Blue, the rest are Red
            State->TeamID = (TotalPlayerCount == 0) ? EPlayerTeam::TEAM_Blue : EPlayerTeam::TEAM_Red;
        }
        else
        {
            // Consecutive games: Winners become Blue, losers become Red
            State->TeamID = (State->Result == EGameResults::RESULT_Won) ? EPlayerTeam::TEAM_Blue : EPlayerTeam::TEAM_Red;
        }

        State->PlayerIndex = TotalPlayerCount++;
        AllPlayers.Add(Cast<APlayerController>(Player));

        // Assign to the correct spawn point based on the team
        if (State->TeamID == EPlayerTeam::TEAM_Blue)
        {
            Start = GetPlayerStart(TEXT("Blue"), -1);
        }
        else
        {
            Start = GetPlayerStart(TEXT("Red"), PlayerStartIndex++);
        }
    }

    return Start;
}

AActor* ANetGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    // Execute custom team logic first
    AActor* Start = AssignTeamAndPlayerStart(Player);
    return Start ? Start : Super::ChoosePlayerStart_Implementation(Player);
}

void ANetGameMode::AvatarsOverlapped(AActor* AvatarA, AActor* AvatarB)
{
    ANetGameState* GState = GetGameState<ANetGameState>();
    if (GState == nullptr || GState->WinningPlayer >= 0) return;

    GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);

    // Blueprint yerine Cast işlemini gizlice C++ içinde yapıyoruz:
    ANetAvatar* NetAvatarA = Cast<ANetAvatar>(AvatarA);
    ANetAvatar* NetAvatarB = Cast<ANetAvatar>(AvatarB);

    // Eğer çarpan şeyler Avatar değilse işlemi iptal et
    if (!NetAvatarA || !NetAvatarB) return;

    ANetPlayerState* StateA = NetAvatarA->GetPlayerState<ANetPlayerState>();
    ANetPlayerState* StateB = NetAvatarB->GetPlayerState<ANetPlayerState>();

    if (!StateA || !StateB || StateA->TeamID == StateB->TeamID) return;

    if (StateA->TeamID == EPlayerTeam::TEAM_Red)
    {
        GState->WinningPlayer = StateA->PlayerIndex;
    }
    else
    {
        GState->WinningPlayer = StateB->PlayerIndex;
    }

    NetAvatarA->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    NetAvatarB->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    GState->OnVictory();

    for (APlayerController* PC : AllPlayers)
    {
        if (PC)
        {
            ANetPlayerState* State = PC->GetPlayerState<ANetPlayerState>();
            if (State)
            {
                if (State->TeamID == EPlayerTeam::TEAM_Blue)
                {
                    State->Result = EGameResults::RESULT_Lost;
                }
                else
                {
                    State->Result = EGameResults::RESULT_Won;
                }
            }
        }
    }

    TotalGames++;

    FTimerHandle EndGameTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(EndGameTimerHandle, this, &ANetGameMode::EndGame, 2.5f, false);
}

void ANetGameMode::EndGame()
{
    PlayerStartIndex = 0;

    ANetGameState* GState = GetGameState<ANetGameState>();
    if (GState)
    {
        GState->WinningPlayer = -1;
    }

    // Reset and respawn all players
    for (APlayerController* PC : AllPlayers)
    {
        if (PC)
        {
            APawn* Pawn = PC->GetPawn();
            if (Pawn)
            {
                PC->UnPossess();
                Pawn->Destroy();
            }
            PC->StartSpot.Reset();
            RestartPlayer(PC);
        }
    }

    if (GState)
    {
        GState->TriggerRestart();
    }
}

void ANetGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Oyun başladığında 30 saniyelik hayatta kalma süresini başlat
    GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &ANetGameMode::OnMatchTimeOut, 30.0f, false);
}

void ANetGameMode::OnMatchTimeOut()
{
    ANetGameState* GState = GetGameState<ANetGameState>();
    // Oyun zaten bir çarpışma ile bittiyse iptal et
    if (GState == nullptr || GState->WinningPlayer >= 0) return;

    int BluePlayerIndex = -1;
    TArray<ANetPlayerState*> RedPlayers;

    // Süre dolduğunda Mavi takım (Kaçan) kazanır. Tüm oyuncuları tara:
    for (APlayerController* PC : AllPlayers)
    {
        if (PC)
        {
            ANetPlayerState* State = PC->GetPlayerState<ANetPlayerState>();
            if (State)
            {
                if (State->TeamID == EPlayerTeam::TEAM_Blue)
                {
                    BluePlayerIndex = State->PlayerIndex;
                    // Mavi takım kazandı AMA gelecek el Kırmızı olması için Result_Lost yapıyoruz (Swap Mantığı)
                    State->Result = EGameResults::RESULT_Lost;
                }
                else
                {
                    RedPlayers.Add(State);
                    State->Result = EGameResults::RESULT_Lost;
                }
            }
        }
    }

    // Kazananı mavi oyuncu olarak ilan et ve UI'yi tetikle
    GState->WinningPlayer = BluePlayerIndex;
    GState->OnVictory();

    // Görev 3: Kırmızı takımdan rastgele birini seçip gelecek el Mavi (Kaçan) yap
    if (RedPlayers.Num() > 0)
    {
        int RandomIndex = FMath::RandRange(0, RedPlayers.Num() - 1);
        RedPlayers[RandomIndex]->Result = EGameResults::RESULT_Won;
    }

    TotalGames++;

    // 2.5 saniye sonra yeni turu başlat
    FTimerHandle EndGameTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(EndGameTimerHandle, this, &ANetGameMode::EndGame, 2.5f, false);
}
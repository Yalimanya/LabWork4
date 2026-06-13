#include "NetBaseCharacter.h"
#include "NetPlayerState.h"
#include "NetGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

static UDataTable* SBodyParts = nullptr;

static const TArray<FString> BodyPartNames = {
    TEXT("Face"),
    TEXT("Hair"),
    TEXT("Chest"),
    TEXT("Hands"),
    TEXT("Legs"),
    TEXT("Beard"),
    TEXT("BodyType")
};

ANetBaseCharacter::ANetBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    PlayerInfoReceived = false;

    PartFace = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Face"));
    PartFace->SetupAttachment(GetMesh());
    PartFace->SetLeaderPoseComponent(GetMesh());

    PartHands = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hands"));
    PartHands->SetupAttachment(GetMesh());
    PartHands->SetLeaderPoseComponent(GetMesh());

    PartLegs = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Legs"));
    PartLegs->SetupAttachment(GetMesh());
    PartLegs->SetLeaderPoseComponent(GetMesh());

    PartHair = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hair"));
    PartHair->SetupAttachment(PartFace, FName("headSocket"));

    PartBeard = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beard"));
    PartBeard->SetupAttachment(PartFace, FName("headSocket"));

    PartEyes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Eyes"));
    PartEyes->SetupAttachment(PartFace, FName("headSocket"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SK_Eyes(TEXT("StaticMesh'/Game/StylizedModularChar/Meshes/SM_Eyes.SM_Eyes'"));
    if (SK_Eyes.Succeeded())
    {
        PartEyes->SetStaticMesh(SK_Eyes.Object);
    }

    static ConstructorHelpers::FObjectFinder<UDataTable> DT_BodyParts(TEXT("DataTable'/Game/Blueprints/DT_BodyParts.DT_BodyParts'"));
    if (DT_BodyParts.Succeeded())
    {
        SBodyParts = DT_BodyParts.Object;
    }

    // Initialize all indices to 0
    for (int i = 0; i < (int)EBodyPart::BP_COUNT; i++)
    {
        BodyPartIndices[i] = 0;
    }
}

void ANetBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Ignore offline modes
    if (GetNetMode() == ENetMode::NM_Standalone) return;

    SetActorHiddenInGame(true);
    CheckPlayerState();
}

void ANetBaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANetBaseCharacter::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    UpdateBodyParts();
}

void ANetBaseCharacter::CheckPlayerState()
{
    ANetPlayerState* State = GetPlayerState<ANetPlayerState>();
    if (State == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("State == nullptr"));
        GetWorldTimerManager().SetTimer(ClientDataCheckTimer, this, &ANetBaseCharacter::CheckPlayerState, 0.25f, false);
    }
    else
    {
        if (IsLocallyControlled())
        {
            UNetGameInstance* Instance = Cast<UNetGameInstance>(GetGameInstance());
            if (Instance)
            {
                SubmitPlayerInfoToServer(Instance->PlayerInfo);
            }
        }
        CheckPlayerInfo();
    }
}

void ANetBaseCharacter::CheckPlayerInfo()
{
    ANetPlayerState* State = GetPlayerState<ANetPlayerState>();
    if (State && PlayerInfoReceived)
    {
        ParseCustomizationData(State->Data.CustomizationData);
        UpdateBodyParts();
        OnPlayerInfoChanged(); // Notify Blueprints
        SetActorHiddenInGame(false);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("State Not Received!"));
        GetWorldTimerManager().SetTimer(ClientDataCheckTimer, this, &ANetBaseCharacter::CheckPlayerInfo, 0.25f, false);
    }
}

void ANetBaseCharacter::SubmitPlayerInfoToServer_Implementation(FSPlayerInfo Info)
{
    ANetPlayerState* State = GetPlayerState<ANetPlayerState>();
    if (State)
    {
        State->Data.Nickname = Info.Nickname;
        State->Data.CustomizationData = Info.CustomizationData;
        State->Data.TeamID = State->TeamID;
    }
    PlayerInfoReceived = true;
}

FString ANetBaseCharacter::GetCustomizationData()
{
    FString Data;
    for (size_t i = 0; i < (int)EBodyPart::BP_COUNT; i++)
    {
        Data += FString::FromInt(BodyPartIndices[i]);
        if (i < ((int)(EBodyPart::BP_COUNT)-1)) Data += TEXT(",");
    }
    return Data;
}

void ANetBaseCharacter::ParseCustomizationData(FString BodyPartData)
{
    TArray<FString> ArrayData;
    BodyPartData.ParseIntoArray(ArrayData, TEXT(","));
    for (int i = 0; i < ArrayData.Num(); i++)
    {
        BodyPartIndices[i] = FCString::Atoi(*ArrayData[i]);
    }
}

FSMeshAssetList* ANetBaseCharacter::GetBodyPartList(EBodyPart part, bool isFemale)
{
    FString Name = FString::Printf(TEXT("%s%s"), isFemale ? TEXT("Female") : TEXT("Male"), *BodyPartNames[(int)part]);
    return SBodyParts ? SBodyParts->FindRow<FSMeshAssetList>(*Name, nullptr) : nullptr;
}

void ANetBaseCharacter::ChangeBodyPart(EBodyPart index, int value, bool DirectSet)
{
    bool isFemale = (BodyPartIndices[(int)EBodyPart::BP_BodyType] != 0);

    // Beard correction for female characters
    if (isFemale && index == EBodyPart::BP_Beard)
    {
        PartBeard->SetStaticMesh(nullptr);
        return;
    }

    FSMeshAssetList* List = GetBodyPartList(index, isFemale);
    if (List == nullptr) return;

    int CurrentIndex = BodyPartIndices[(int)index];

    if (DirectSet)
    {
        CurrentIndex = value;
    }
    else
    {
        CurrentIndex += value;
        int Num = List->ListSkeletal.Num() + List->ListStatic.Num();
        if (Num == 0) return;

        if (CurrentIndex < 0)
        {
            CurrentIndex += Num;
        }
        else
        {
            CurrentIndex %= Num;
        }
    }

    BodyPartIndices[(int)index] = CurrentIndex;

    switch (index)
    {
    case EBodyPart::BP_Face:
        if (List->ListSkeletal.IsValidIndex(CurrentIndex)) PartFace->SetSkeletalMeshAsset(List->ListSkeletal[CurrentIndex]);
        break;
    case EBodyPart::BP_Beard:
        if (List->ListStatic.IsValidIndex(CurrentIndex)) PartBeard->SetStaticMesh(List->ListStatic[CurrentIndex]);
        break;
    case EBodyPart::BP_Chest:
        if (List->ListSkeletal.IsValidIndex(CurrentIndex)) GetMesh()->SetSkeletalMeshAsset(List->ListSkeletal[CurrentIndex]);
        break;
    case EBodyPart::BP_Hair:
        if (List->ListStatic.IsValidIndex(CurrentIndex)) PartHair->SetStaticMesh(List->ListStatic[CurrentIndex]);
        break;
    case EBodyPart::BP_Hands:
        if (List->ListSkeletal.IsValidIndex(CurrentIndex)) PartHands->SetSkeletalMeshAsset(List->ListSkeletal[CurrentIndex]);
        break;
    case EBodyPart::BP_Legs:
        if (List->ListSkeletal.IsValidIndex(CurrentIndex)) PartLegs->SetSkeletalMeshAsset(List->ListSkeletal[CurrentIndex]);
        break;
    default:
        break;
    }
}

void ANetBaseCharacter::ChangeGender(bool isFemale)
{
    BodyPartIndices[(int)EBodyPart::BP_BodyType] = isFemale ? 1 : 0;
    UpdateBodyParts();
}

void ANetBaseCharacter::UpdateBodyParts()
{
    ChangeBodyPart(EBodyPart::BP_Face, 0, false);
    ChangeBodyPart(EBodyPart::BP_Beard, 0, false);
    ChangeBodyPart(EBodyPart::BP_Chest, 0, false);
    ChangeBodyPart(EBodyPart::BP_Hair, 0, false);
    ChangeBodyPart(EBodyPart::BP_Hands, 0, false);
    ChangeBodyPart(EBodyPart::BP_Legs, 0, false);
}

void ANetBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANetBaseCharacter, bIsRunning);
}

void ANetBaseCharacter::StartRunning()
{
    if (!HasAuthority())
    {
        ServerSetRunning(true);
    }
    bIsRunning = true;
    OnRep_IsRunning();
}

void ANetBaseCharacter::StopRunning()
{
    if (!HasAuthority())
    {
        ServerSetRunning(false);
    }
    bIsRunning = false;
    OnRep_IsRunning();
}

bool ANetBaseCharacter::ServerSetRunning_Validate(bool bNewRunning)
{
    return true;
}

void ANetBaseCharacter::ServerSetRunning_Implementation(bool bNewRunning)
{
    bIsRunning = bNewRunning;
    OnRep_IsRunning();
}

void ANetBaseCharacter::OnRep_IsRunning()
{
    if (bIsRunning)
    {
        GetCharacterMovement()->MaxWalkSpeed = 600.0f;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    }
}
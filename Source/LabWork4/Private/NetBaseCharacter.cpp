#include "NetBaseCharacter.h"
#include "NetGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

// Pointer to our DataTable
static UDataTable* SBodyParts = nullptr;

// Mapping enums to row names in the DataTable
static const TArray<FString> BodyPartNames = {
    TEXT("Face"),
    TEXT("Hair"),
    TEXT("Chest"),
    TEXT("Hands"),
    TEXT("Legs"),
    TEXT("Beard"),
    TEXT("Eyebrows") // Added for Assignment #2
};

ANetBaseCharacter::ANetBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize Face
    PartFace = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Face"));
    PartFace->SetupAttachment(GetMesh());
    PartFace->SetLeaderPoseComponent(GetMesh());

    // Initialize Hands
    PartHands = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hands"));
    PartHands->SetupAttachment(GetMesh());
    PartHands->SetLeaderPoseComponent(GetMesh());

    // Initialize Legs
    PartLegs = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Legs"));
    PartLegs->SetupAttachment(GetMesh());
    PartLegs->SetLeaderPoseComponent(GetMesh());

    // Initialize Hair
    PartHair = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hair"));
    PartHair->SetupAttachment(PartFace, FName("headSocket"));

    // Initialize Beard
    PartBeard = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beard"));
    PartBeard->SetupAttachment(PartFace, FName("headSocket"));

    // Initialize Eyebrows (Added for Assignment #2)
    PartEyebrows = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Eyebrows"));
    PartEyebrows->SetupAttachment(PartFace, FName("headSocket"));

    // Initialize Eyes
    PartEyes = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Eyes"));
    PartEyes->SetupAttachment(PartFace, FName("headSocket"));

    // Load default eye mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SK_Eyes(TEXT("StaticMesh'/Game/StylizedModularChar/Meshes/SM_Eyes.SM_Eyes'"));
    if (SK_Eyes.Succeeded())
    {
        PartEyes->SetStaticMesh(SK_Eyes.Object);
    }

    // Load DataTable containing customization parts
    static ConstructorHelpers::FObjectFinder<UDataTable> DT_BodyParts(TEXT("DataTable'/Game/Blueprints/DT_BodyParts.DT_BodyParts'"));
    if (DT_BodyParts.Succeeded())
    {
        SBodyParts = DT_BodyParts.Object;
    }
}

void ANetBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Send local player info to the server upon spawning
    if (IsLocallyControlled())
    {
        UNetGameInstance* Instance = Cast<UNetGameInstance>(GetGameInstance());
        if (Instance && Instance->PlayerInfo.Ready)
        {
            SubmitPlayerInfoToServer(Instance->PlayerInfo);
        }
    }
}

void ANetBaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANetBaseCharacter::OnConstruction(const FTransform& Transform)
{
    UpdateBodyParts();
}

FSMeshAssetList* ANetBaseCharacter::GetBodyPartList(EBodyPart part, bool isFemale)
{
    // Construct the row name (e.g., "MaleFace" or "FemaleHair")
    FString Name = FString::Printf(TEXT("%s%s"), isFemale ? TEXT("Female") : TEXT("Male"), *BodyPartNames[(int)part]);
    return SBodyParts ? SBodyParts->FindRow<FSMeshAssetList>(*Name, nullptr) : nullptr;
}

void ANetBaseCharacter::ChangeBodyPart(EBodyPart index, int value, bool DirectSet)
{
    // Assignment #2: Beard correction for female characters
    if (PartSelection.isFemale && index == EBodyPart::BP_Beard)
    {
        PartBeard->SetStaticMesh(nullptr);
        return;
    }

    FSMeshAssetList* List = GetBodyPartList(index, PartSelection.isFemale);
    if (List == nullptr) return;

    int CurrentIndex = PartSelection.Indices[(int)index];

    // Calculate new index based on DirectSet or relative increment/decrement
    if (DirectSet)
    {
        CurrentIndex = value;
    }
    else
    {
        CurrentIndex += value;
        int Num = List->ListSkeletal.Num() + List->ListStatic.Num();

        if (Num == 0) return; // Prevent division by zero

        // Wrap around logic
        if (CurrentIndex < 0)
        {
            CurrentIndex += Num;
        }
        else
        {
            CurrentIndex %= Num;
        }
    }

    PartSelection.Indices[(int)index] = CurrentIndex;

    // Apply the mesh based on the selected body part
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
    case EBodyPart::BP_Eyebrows: // Assignment #2
        if (List->ListStatic.IsValidIndex(CurrentIndex)) PartEyebrows->SetStaticMesh(List->ListStatic[CurrentIndex]);
        break;
    default:
        break;
    }
}

void ANetBaseCharacter::ChangeGender(bool isFemale)
{
    PartSelection.isFemale = isFemale;
    UpdateBodyParts();
}

void ANetBaseCharacter::UpdateBodyParts()
{
    // Force an update on all body parts
    ChangeBodyPart(EBodyPart::BP_Face, 0, false);
    ChangeBodyPart(EBodyPart::BP_Beard, 0, false);
    ChangeBodyPart(EBodyPart::BP_Chest, 0, false);
    ChangeBodyPart(EBodyPart::BP_Hair, 0, false);
    ChangeBodyPart(EBodyPart::BP_Hands, 0, false);
    ChangeBodyPart(EBodyPart::BP_Legs, 0, false);
    ChangeBodyPart(EBodyPart::BP_Eyebrows, 0, false); // Assignment #2
}

void ANetBaseCharacter::SubmitPlayerInfoToServer_Implementation(FSPlayerInfo Info)
{
    PartSelection = Info.BodyParts;

    if (HasAuthority())
    {
        // Trigger replication to all clients
        OnRep_PlayerInfoChanged();
    }
}

void ANetBaseCharacter::OnRep_PlayerInfoChanged()
{
    UpdateBodyParts();
}

void ANetBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Register properties for replication
    DOREPLIFETIME(ANetBaseCharacter, PartSelection);
    DOREPLIFETIME(ANetBaseCharacter, bIsRunning); // Added for Assignment #2
}

// --- RUNNING MECHANICS (Assignment #2) ---

void ANetBaseCharacter::StartRunning()
{
    // If client, send request to server
    if (!HasAuthority())
    {
        ServerSetRunning(true);
    }

    bIsRunning = true;
    OnRep_IsRunning(); // Predict locally immediately
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
    return true; // Simple validation pass
}

void ANetBaseCharacter::ServerSetRunning_Implementation(bool bNewRunning)
{
    bIsRunning = bNewRunning;
    OnRep_IsRunning(); // Apply changes on the server side
}

void ANetBaseCharacter::OnRep_IsRunning()
{
    // Adjust speed based on the replicated boolean
    if (bIsRunning)
    {
        GetCharacterMovement()->MaxWalkSpeed = 600.0f; // Run speed
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = 300.0f; // Walk speed
    }
}
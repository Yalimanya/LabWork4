#include "NetAvatar.h"
#include "GameFramework/CharacterMovementComponent.h"

ANetAvatar::ANetAvatar()
{
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void ANetAvatar::BeginPlay()
{
    Super::BeginPlay();

    Camera->bUsePawnControlRotation = false;
    SpringArm->bUsePawnControlRotation = true;
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ANetAvatar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("Turn", this, &ACharacter::AddControllerYawInput);
    PlayerInputComponent->BindAxis("Lookup", this, &ACharacter::AddControllerPitchInput);

    PlayerInputComponent->BindAxis("MoveForward", this, &ANetAvatar::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ANetAvatar::MoveRight);

    // Assignment #2: Koşma (Run) eylemi bağlamaları
    PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ANetBaseCharacter::StartRunning);
    PlayerInputComponent->BindAction("Run", IE_Released, this, &ANetBaseCharacter::StopRunning);
}

void ANetAvatar::MoveForward(float Scale)
{
    FRotator Rotation = GetController()->GetControlRotation();
    FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
    FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    AddMovementInput(ForwardDirection, Scale);
}

void ANetAvatar::MoveRight(float Scale)
{
    FRotator Rotation = GetController()->GetControlRotation();
    FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
    FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    AddMovementInput(RightDirection, Scale);
}


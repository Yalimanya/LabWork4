#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "NetBaseCharacter.generated.h"


UENUM(BlueprintType)
enum class EBodyPart : uint8
{
    BP_Face = 0,
    BP_Hair = 1,
    BP_Chest = 2,
    BP_Hands = 3,
    BP_Legs = 4,
    BP_Beard = 5,
    BP_Eyebrows = 6, // Yeni Eklenen Yuva
    BP_COUNT = 7     // Toplam 7 oldu
};

// 2. Data Table için Satır Yapısı
USTRUCT(BlueprintType)
struct FSMeshAssetList : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<USkeletalMesh*> ListSkeletal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMesh*> ListStatic;
};

// 3. Karakterin Seçtiği Parçaların İndeksleri ve Cinsiyet
USTRUCT(BlueprintType)
struct FSBodyPartSelection
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    int Indices[(int)EBodyPart::BP_COUNT];

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool isFemale;
};

// 4. Ağ Üzerinde Taşınacak Ana Oyuncu Verisi
USTRUCT(BlueprintType)
struct FSPlayerInfo
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Nickname;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSBodyPartSelection BodyParts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool Ready;
};

UCLASS()
class LABWORK4_API ANetBaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ANetBaseCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void OnConstruction(const FTransform& Transform) override;

    // --- ÖZELLEŞTİRME (CUSTOMIZATION) FONKSİYONLARI ---
    UFUNCTION(BlueprintCallable)
    void ChangeBodyPart(EBodyPart index, int value, bool DirectSet);

    UFUNCTION(BlueprintCallable)
    void ChangeGender(bool isFemale);

    void UpdateBodyParts();

    static FSMeshAssetList* GetBodyPartList(EBodyPart part, bool isFemale);

    // --- AĞ (REPLICATION) DEĞİŞKENLERİ VE FONKSİYONLARI ---
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerInfoChanged)
    FSBodyPartSelection PartSelection;

    UFUNCTION(Server, Reliable)
    void SubmitPlayerInfoToServer(FSPlayerInfo Info);

    UFUNCTION()
    void OnRep_PlayerInfoChanged();

    // --- KOŞMA (RUN) MEKANİĞİ ---
    UPROPERTY(Transient, ReplicatedUsing = OnRep_IsRunning)
    bool bIsRunning;

    UFUNCTION()
    void OnRep_IsRunning();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetRunning(bool bNewRunning);

    void StartRunning();
    void StopRunning();

private:
    // --- VÜCUT PARÇASI BİLEŞENLERİ (COMPONENTS) ---
    UPROPERTY()
    USkeletalMeshComponent* PartFace;

    UPROPERTY()
    UStaticMeshComponent* PartHair;

    UPROPERTY()
    UStaticMeshComponent* PartBeard;

    UPROPERTY()
    UStaticMeshComponent* PartEyes;

    UPROPERTY()
    USkeletalMeshComponent* PartHands;

    UPROPERTY()
    USkeletalMeshComponent* PartLegs;

    UPROPERTY()
    UStaticMeshComponent* PartEyebrows; // Kaşlar
};
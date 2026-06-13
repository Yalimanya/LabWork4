#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetGameInstance.h" // FSPlayerInfo için eklendi
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"
#include "NetBaseCharacter.generated.h"

// 1. Enum Güncellendi (IsFemale yerine BP_BodyType eklendi) 
UENUM(BlueprintType)
enum class EBodyPart : uint8
{
    BP_Face = 0,
    BP_Hair = 1,
    BP_Chest = 2,
    BP_Hands = 3,
    BP_Legs = 4,
    BP_Beard = 5,
    BP_BodyType = 6, // Yeni eklenen
    BP_COUNT = 7
};

// 2. Data Table Struct'ı aynı kalıyor 
USTRUCT(BlueprintType)
struct FSMeshAssetList : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<USkeletalMesh*> ListSkeletal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMesh*> ListStatic;
};

UCLASS()
class LABWORK4_API ANetBaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ANetBaseCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- PHASE 2: YENİ ÖZELLEŞTİRME (CUSTOMIZATION) FONKSİYONLARI --- 
    UFUNCTION(BlueprintPure)
    FString GetCustomizationData();

    void ParseCustomizationData(FString Data);

    UFUNCTION(BlueprintCallable)
    void ChangeBodyPart(EBodyPart index, int value, bool DirectSet);

    UFUNCTION(BlueprintCallable)
    void ChangeGender(bool isFemale);

    UFUNCTION(Server, Reliable)
    void SubmitPlayerInfoToServer(FSPlayerInfo Info);

    // Blueprint tarafında takım renklerini (Kırmızı/Mavi) ayarlamak için kullanılacak 
    UFUNCTION(BlueprintImplementableEvent)
    void OnPlayerInfoChanged();

    // PlayerState ve Info'nun gelmesini beklemek için Timer fonksiyonları 
    UFUNCTION()
    void CheckPlayerState();

    UFUNCTION()
    void CheckPlayerInfo();

    // --- VÜCUT PARÇASI BİLEŞENLERİ (COMPONENTS) --- 
    // EditAnywhere ve BlueprintReadWrite yapılarak Blueprint'e açıldılar
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMeshComponent* PartFace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* PartHair;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* PartBeard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* PartEyes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMeshComponent* PartHands;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMeshComponent* PartLegs;

    // Assignment 1'den kalan kaşlar (Eğer hata verirse silebilirsin, Phase 2'de zorunlu değil)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* PartEyebrows;

    // Verinin tam olarak alınıp alınmadığını kontrol eden bayrak 
    bool PlayerInfoReceived;

    // --- KOŞMA (RUN) MEKANİĞİ (Assignment 2 - Phase 1'den kalma) ---
    UPROPERTY(Transient, ReplicatedUsing = OnRep_IsRunning)
    bool bIsRunning;

    UFUNCTION()
    void OnRep_IsRunning();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetRunning(bool bNewRunning);

    void StartRunning();
    void StopRunning();

private:
    // --- PHASE 2: YENİ VERİ DEPOLAMA YÖNTEMİ --- 
    // Artık struct yerine bu diziyi kullanıyoruz
    int BodyPartIndices[(int)EBodyPart::BP_COUNT];

    void UpdateBodyParts();
    static FSMeshAssetList* GetBodyPartList(EBodyPart part, bool isFemale);

    // Ağdan gelen veriyi belirli aralıklarla kontrol etmek için zamanlayıcı 
    FTimerHandle ClientDataCheckTimer;
};
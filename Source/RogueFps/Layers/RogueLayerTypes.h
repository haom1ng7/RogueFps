// RogueLayerTypes.h

#pragma once

#include "CoreMinimal.h"
#include "RogueLayerTypes.generated.h"

UENUM(BlueprintType)
enum class ERoomType : uint8
{
    Battle      UMETA(DisplayName = "Battle"),
    BattleHard  UMETA(DisplayName = "BattleHard"),
    Event       UMETA(DisplayName = "Event"),
    Shop        UMETA(DisplayName = "Shop"),
    Boss        UMETA(DisplayName = "Boss")
};

USTRUCT(BlueprintType)
struct FRoomNode
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Id = -1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Depth = 0;      // 1..8
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) ERoomType Type = ERoomType::Battle;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TSoftObjectPtr<UWorld> LevelRef;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 DangerTier = 1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName Tag;
};

USTRUCT(BlueprintType)
struct FRoomColumn
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<FRoomNode> Nodes; // 当前Depth的候选集合
};

USTRUCT(BlueprintType)
struct FRogueLayerGraph
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TMap<int32, FRoomColumn> Columns; // Key=Depth
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 ShopNodeId = -1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 BossNodeId = -1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 SeedUsed = 0;
};

USTRUCT(BlueprintType)
struct FRogueLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Steps = 8;              // 固定 8
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HardBattleChance = 0.15f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUniqueEventsPerLayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<class URoomArchetypeDA*> BattlePool;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<class URoomArchetypeDA*> BattleHardPool;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<class URoomArchetypeDA*> EventPool;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<class URoomArchetypeDA*> ShopPool;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<class URoomArchetypeDA*> BossPool;
};

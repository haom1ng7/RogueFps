// RunStateSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Layers/RogueLayerTypes.h"
#include "Rooms/RoomArchetypeDA.h"
#include "RogueMapGenerator.h"
#include "RunStateSubsystem.generated.h"

class URogueMapGenerator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDepthAdvanced, int32, NewDepth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLayerStarted, int32, SeedUsed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNodePicked, int32, NodeId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLayerFinished);

UCLASS()
class ROGUEFPS_API URunStateSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    void StartLayer(const FRogueLayerConfig& Config, int32 Seed);

    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    int32 GetCurrentDepth() const { return CurrentDepth; }

    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    const FRogueLayerGraph& GetGraph() const { return Graph; }

    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    TArray<FRoomNode> GetChoicesForDepth(int32 Depth) const;

    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    TArray<FRoomNode> GetChoicesForCurrentDepth() const { return GetChoicesForDepth(CurrentDepth); }

    // 在 Depth2~6 的选择界面中调用；Depth1/7/8不会调用（它们只有一个候选）
    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    void PickNodeForCurrentDepth(int32 NodeId);

    // 某房间完成时由关卡调用
    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    void NotifyRoomCompleted(int32 CompletedNodeId);

    // 查询/辅助
    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    bool TryGetNodeById(int32 NodeId, FRoomNode& OutNode) const;

    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    bool GetPickedNodeForDepth(int32 Depth, FRoomNode& OutNode) const;

    UFUNCTION(BlueprintCallable, Category = "Rogue|Run")
    const TArray<int32>& GetChosenPath() const { return ChosenPath; }

    // 事件
    UPROPERTY(BlueprintAssignable) FOnLayerStarted OnLayerStarted;
    UPROPERTY(BlueprintAssignable) FOnDepthAdvanced OnDepthAdvanced;
    UPROPERTY(BlueprintAssignable) FOnNodePicked OnNodePicked;
    UPROPERTY(BlueprintAssignable) FOnLayerFinished OnLayerFinished;

private:
    FRogueLayerGraph Graph;
    FRogueLayerConfig CachedConfig;

    int32 CurrentDepth = 1;
    // 记录“某深度→选了哪个NodeId”
    TMap<int32, int32> DepthToPickedNodeId;
    // 玩家最终走的路径（按顺序，便于存档/回放）
    TArray<int32> ChosenPath;

    bool FindNodeById_Internal(int32 NodeId, FRoomNode& OutNode) const;
    void BroadcastDepth(int32 Depth);
};

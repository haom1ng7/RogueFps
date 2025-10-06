// RunStateSubsystem.cpp

#include "RunStateSubsystem.h"

void URunStateSubsystem::StartLayer(const FRogueLayerConfig& Config, int32 Seed)
{
    CachedConfig = Config;
    CurrentDepth = 1;
    DepthToPickedNodeId.Reset();
    ChosenPath.Reset();

    URogueMapGenerator* Gen = NewObject<URogueMapGenerator>(this);
    Graph = Gen->GenerateLayer(Config, Seed);

    // Depth1 只有1个候选：直接标记为已选
    if (const FRoomColumn* Col = Graph.Columns.Find(1))
    {
        if (Col->Nodes.Num() > 0)
        {
            const int32 NodeId = Col->Nodes[0].Id;
            DepthToPickedNodeId.Add(1, NodeId);
            ChosenPath.Add(NodeId);
        }
    }

    OnLayerStarted.Broadcast(Graph.SeedUsed);
    BroadcastDepth(CurrentDepth); // OnDepthAdvanced(1)
}

TArray<FRoomNode> URunStateSubsystem::GetChoicesForDepth(int32 Depth) const
{
    if (const FRoomColumn* Col = Graph.Columns.Find(Depth))
        return Col->Nodes;
    return {};
}

void URunStateSubsystem::PickNodeForCurrentDepth(
    int32 NodeId)
{
    // 只能在 2..6 调用（有双选），其它深度忽略
    if (CurrentDepth < 2 || CurrentDepth > 6) return;

    // 验证这个 NodeId 是当前Depth的候选之一
    bool bValid = false;
    if (const FRoomColumn* Col = Graph.Columns.Find(CurrentDepth))
    {
        for (const FRoomNode& N : Col->Nodes) if (N.Id == NodeId) { bValid = true; break; }
    }
    if (!bValid) { UE_LOG(LogTemp, Warning, TEXT("PickNodeForCurrentDepth: invalid NodeId %d at Depth %d"), NodeId, CurrentDepth); return; }

    DepthToPickedNodeId.Add(CurrentDepth, NodeId);
    ChosenPath.Add(NodeId);

    OnNodePicked.Broadcast(NodeId); // 让关卡加载器据此加载
    // 注意：不在这里推进Depth。等房间完成后推进。
}

void URunStateSubsystem::NotifyRoomCompleted(int32 CompletedNodeId)
{
    FRoomNode Node;
    if (!FindNodeById_Internal(CompletedNodeId, Node))
    {
        UE_LOG(LogTemp, Warning, TEXT("NotifyRoomCompleted: NodeId %d not found"), CompletedNodeId);
        return;
    }

    // 防呆：完成的必须是当前Depth的“已选节点”
    int32* PPickedId = DepthToPickedNodeId.Find(CurrentDepth);
    if (!PPickedId || *PPickedId != CompletedNodeId)
    {
        UE_LOG(LogTemp, Warning, TEXT("NotifyRoomCompleted: NodeId %d is not picked at Depth %d"), CompletedNodeId, CurrentDepth);
        return;
    }

    // Boss（8）完成 → 层结束
    if (CurrentDepth >= 8)
    {
        OnLayerFinished.Broadcast();
        return;
    }

    // 推进到下一深度
    CurrentDepth++;
    BroadcastDepth(CurrentDepth); // OnDepthAdvanced(2..8)

    // 深度=7/8 时，只有一个候选：在关卡加载器里会直接加载，无需Pick
}

bool URunStateSubsystem::TryGetNodeById(int32 NodeId, FRoomNode& OutNode) const
{
    return FindNodeById_Internal(NodeId, OutNode);
}

bool URunStateSubsystem::GetPickedNodeForDepth(int32 Depth, FRoomNode& OutNode) const
{
    if (const int32* PId = DepthToPickedNodeId.Find(Depth))
        return FindNodeById_Internal(*PId, OutNode);
    return false;
}

bool URunStateSubsystem::FindNodeById_Internal(int32 NodeId, FRoomNode& OutNode) const
{
    for (const auto& Kvp : Graph.Columns)
        for (const FRoomNode& N : Kvp.Value.Nodes)
            if (N.Id == NodeId) { OutNode = N; return true; }
    return false;
}

void URunStateSubsystem::BroadcastDepth(int32 Depth)
{
    OnDepthAdvanced.Broadcast(Depth);
    // 对于单选深度（1/7/8），我们也提前把“已选”抛给加载器，便于它直接加载
    if (Depth == 1 || Depth == 7 || Depth == 8)
    {
        FRoomNode N;
        if (const FRoomColumn* Col = Graph.Columns.Find(Depth))
        {
            if (Col->Nodes.Num() > 0)
            {
                const int32 NodeId = Col->Nodes[0].Id;
                DepthToPickedNodeId.FindOrAdd(Depth) = NodeId;
                // Depth1 已在 StartLayer 填过，但这里统一一次
                OnNodePicked.Broadcast(NodeId);
            }
        }
    }
}

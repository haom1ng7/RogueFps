// LevelFlowManager.cpp

#include "LevelFlowManager.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "RunStateSubsystem.h"

ALevelFlowManager::ALevelFlowManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ALevelFlowManager::BeginPlay()
{
    Super::BeginPlay();

    RunState = GetGameInstance()->GetSubsystem<URunStateSubsystem>();
    if (!RunState) { UE_LOG(LogTemp, Error, TEXT("RunStateSubsystem not found")); return; }

    RunState->OnDepthAdvanced.AddDynamic(this, &ALevelFlowManager::HandleDepthAdvanced);
    RunState->OnNodePicked.AddDynamic(this, &ALevelFlowManager::HandleNodePicked);
    RunState->OnLayerFinished.AddDynamic(this, &ALevelFlowManager::HandleLayerFinished);

    // 如果主房间是空的则删掉注释内容 直接开始游戏
    StartNextFloor();
}

static int32 MakeRandomRunSeed()
{
    // 多源熵混合，避免总是一样
    const int32 t1 = (int32)FDateTime::UtcNow().GetTicks();
    const int32 t2 = (int32)FPlatformTime::Cycles();
    const int32 t3 = FMath::Rand(); // 引擎全局 RNG
    return t1 ^ (t2 << 1) ^ (t3 << 2);
}



void ALevelFlowManager::StartNextFloor()
{
    if (CurrentFloor >= TotalFloors)
    {
        UE_LOG(LogTemp, Log, TEXT("All floors finished! Show result/return HUB."));
        return;
    }

    SeedBase = bUseRandomSeedPerRun ? MakeRandomRunSeed() : SeedBase;

    const int32 FloorIdx = CurrentFloor; // 0-based
    const FRogueLayerConfig& Cfg =
        FloorConfigs.IsValidIndex(FloorIdx) ? FloorConfigs[FloorIdx] : FloorConfigs.Last();

    const int32 Seed = SeedBase + FloorIdx; // 每层不同种子
    RunState->StartLayer(Cfg, Seed); // 生成并推进到Depth=1
    CurrentFloor++;
}

void ALevelFlowManager::HandleDepthAdvanced(int32 NewDepth)
{
    // 深度变化只决定“是否需要选择”；真正加载发生在 OnNodePicked
    // 对于单选深度（1/7/8），子系统会立即广播 OnNodePicked，所以这里在深度变化后给新房间广播让他们提前准备选项（
    UE_LOG(LogTemp, Log, TEXT("Depth advanced to %d"), NewDepth);
}

void ALevelFlowManager::HandleNodePicked(int32 NodeId)
{
    FRoomNode Node;
    if (!RunState || !RunState->TryGetNodeById(NodeId, Node)) return;

    // 切换加载
    LoadRoomByNode(Node);
}

void ALevelFlowManager::HandleLayerFinished()
{
    UE_LOG(LogTemp, Log, TEXT("Layer finished!"));
    UnloadCurrentRoom();
    StartNextFloor(); // 这一层打完Boss -> 开下一层或收尾
}

bool ALevelFlowManager::ResolveWorldFromSoftRef(const TSoftObjectPtr<UWorld>& SoftWorld, FString& OutLevelName)
{
    if (!SoftWorld.IsValid())
    {
        const UWorld* W = SoftWorld.LoadSynchronous(); // 小图可同步；大图建议异步
        if (!W) return false;
    }
    OutLevelName = SoftWorld.GetAssetName();
    return true;
}

void ALevelFlowManager::LoadRoomByNode(const FRoomNode& Node)
{
    // 先卸载上一个
    UnloadCurrentRoom();

    if (!Node.LevelRef.IsValid() && Node.LevelRef.ToSoftObjectPath().IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("LoadRoomByNode: Node %d has invalid LevelRef"), Node.Id);
        return;
    }

    const FVector Loc = FVector::ZeroVector;
    const FRotator Rot = FRotator::ZeroRotator;

    bool bSuccess = false;
    ULevelStreamingDynamic* Streaming = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
        /*WorldContextObject=*/this,        // 注意要传 UObject*，传 this 最方便
        /*Level=*/Node.LevelRef,
        /*Location=*/Loc,
        /*Rotation=*/Rot,
        /*out*/ bSuccess
    );

    if (!bSuccess || !Streaming)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadLevelInstanceBySoftObjectPtr failed for %s"),
            *Node.LevelRef.ToSoftObjectPath().ToString());
        return;
    }

    CurrentStreaming = Streaming;
    CurrentNodeId = Node.Id;

    UE_LOG(LogTemp, Log, TEXT("Loaded Node %d (Depth %d) from %s"),
        Node.Id, Node.Depth, *Node.LevelRef.ToSoftObjectPath().ToString());
}

void ALevelFlowManager::UnloadCurrentRoom()
{
    if (CurrentStreaming)
    {
        CurrentStreaming->SetIsRequestingUnloadAndRemoval(true);
        CurrentStreaming = nullptr;
        CurrentNodeId = -1;
    }
}
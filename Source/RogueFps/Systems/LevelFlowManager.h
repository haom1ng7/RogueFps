// LevelFlowManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Layers/RogueLayerTypes.h"
#include "LevelFlowManager.generated.h"

class URunStateSubsystem;
class ULevelStreamingDynamic;

UCLASS()
class ROGUEFPS_API ALevelFlowManager : public AActor
{
    GENERATED_BODY()
public:
    ALevelFlowManager();

protected:
    virtual void BeginPlay() override;

    UFUNCTION() void HandleDepthAdvanced(int32 NewDepth);
    UFUNCTION() void HandleNodePicked(int32 NodeId);
    UFUNCTION() void HandleLayerFinished();

    // 真正加载/卸载
    void LoadRoomByNode(const FRoomNode& Node);
    void UnloadCurrentRoom();

    // 工具：通过软引用加载关卡
    bool ResolveWorldFromSoftRef(const TSoftObjectPtr<UWorld>& SoftWorld, FString& OutLevelName);

    UPROPERTY(EditAnywhere, Category = "Rogue|Run")
    int32 TotalFloors = 2;

    UPROPERTY(VisibleAnywhere, Category = "Rogue|Run")
    int32 CurrentFloor = 0;

    // 每层的生成配置（可在蓝图里设置不同池子/不同参数）
    UPROPERTY(EditAnywhere, Category = "Rogue|Run")
    TArray<FRogueLayerConfig> FloorConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rogue|Run")
    bool bUseRandomSeedPerRun = true;          // 默认为“每局随机”

    UPROPERTY(EditAnywhere, Category = "Rogue|Run")
    int32 SeedBase;

    UFUNCTION() void StartNextFloor();

private:
    UPROPERTY() URunStateSubsystem* RunState = nullptr;
    UPROPERTY() ULevelStreamingDynamic* CurrentStreaming = nullptr;
    UPROPERTY(VisibleAnywhere) int32 CurrentNodeId = -1;
};

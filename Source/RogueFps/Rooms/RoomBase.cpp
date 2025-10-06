// RoomBase.cpp

#include "RoomBase.h"
#include "Systems/RunStateSubsystem.h"
#include "Engine/GameInstance.h"

ARoomBase::ARoomBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARoomBase::BeginPlay()
{
    Super::BeginPlay();
}

void ARoomBase::PushState(ERoomState NewState)
{
    State = NewState;
    OnRoomStateChanged.Broadcast(State); // 蓝图可监听房间状态变化
}

void ARoomBase::InitializeRoom(int32 InNodeId, int32 InFloorIndex)
{
    if (State != ERoomState::None) return;
    NodeId = InNodeId; FloorIndex = InFloorIndex;
    PushState(ERoomState::Initialized);
    OnRoomInitialized();
}

void ARoomBase::ActivateRoom()
{
    if (State != ERoomState::Initialized) return;
    PushState(ERoomState::Active);
    OnRoomActivated();
}

void ARoomBase::DeactivateRoom()
{
    OnRoomDeactivated();
}

void ARoomBase::MarkCompleted()
{
    if (State != ERoomState::Active) return;
    PushState(ERoomState::Completed);
    OnRoomCompleted();

    if (UGameInstance* GI = GetGameInstance())
        if (auto* Run = GI->GetSubsystem<URunStateSubsystem>())
            Run->NotifyRoomCompleted(NodeId); // 推进到下一深度/下一层
}

void ARoomBase::MarkFailed()
{
    if (State != ERoomState::Active) return;
    PushState(ERoomState::Failed);
    OnRoomFailed();
    // 失败后的处理按你规则再决定（回层起点/扣命等）
}

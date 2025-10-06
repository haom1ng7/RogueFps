// RoomBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomBase.generated.h"

class URunStateSubsystem;

UENUM(BlueprintType)
enum class ERoomState : uint8 { None, Initialized, Active, Completed, Failed };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomStateChanged, ERoomState, NewState);

UCLASS(Abstract, Blueprintable)
class ARoomBase : public AActor
{
    GENERATED_BODY()
public:
    ARoomBase();

    UFUNCTION(BlueprintCallable, Category = "Room")
    void InitializeRoom(int32 InNodeId, int32 InFloorIndex);

    UFUNCTION(BlueprintCallable, Category = "Room")
    void ActivateRoom();

    UFUNCTION(BlueprintCallable, Category = "Room")
    void DeactivateRoom();

    UFUNCTION(BlueprintCallable, Category = "Room")
    void MarkCompleted();

    UFUNCTION(BlueprintCallable, Category = "Room")
    void MarkFailed();

    UFUNCTION(BlueprintPure, Category = "Room")
    FORCEINLINE int32 GetNodeId() const { return NodeId; }

    UFUNCTION(BlueprintPure, Category = "Room")
    FORCEINLINE int32 GetFloorIndex() const { return FloorIndex; }

    UPROPERTY(BlueprintAssignable, Category = "Room")
    FOnRoomStateChanged OnRoomStateChanged;

protected:
    virtual void BeginPlay() override;

    // 让房间子蓝图实现具体行为（开刷/关门/掉落等）
    UFUNCTION(BlueprintImplementableEvent, Category = "Room") void OnRoomInitialized();
    UFUNCTION(BlueprintImplementableEvent, Category = "Room") void OnRoomActivated();
    UFUNCTION(BlueprintImplementableEvent, Category = "Room") void OnRoomDeactivated();
    UFUNCTION(BlueprintImplementableEvent, Category = "Room") void OnRoomCompleted();
    UFUNCTION(BlueprintImplementableEvent, Category = "Room") void OnRoomFailed();

    UPROPERTY(BlueprintReadOnly, Category = "Room")
    int32 NodeId = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Room")
    int32 FloorIndex = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Room")
    ERoomState State = ERoomState::None;

private:
    void PushState(ERoomState NewState);
};

// RoomArchetypeDA.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Layers/RogueLayerTypes.h"
#include "RoomArchetypeDA.generated.h"

UCLASS(BlueprintType)
class ROGUEFPS_API URoomArchetypeDA : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) ERoomType RoomType = ERoomType::Battle;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TSoftObjectPtr<UWorld> LevelToStream;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1")) int32 DangerTier = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1")) int32 Weight = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName Tag;
};

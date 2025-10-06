// RogueMapGenerator.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Layers/RogueLayerTypes.h"
#include "Rooms/RoomArchetypeDA.h"
#include "RogueMapGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRogueMapGen, Log, All);

UCLASS(BlueprintType)
class ROGUEFPS_API URogueMapGenerator : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Rogue|MapGen")
    FRogueLayerGraph GenerateLayer(const FRogueLayerConfig& Config, int32 Seed);

private:
    URoomArchetypeDA* WeightedPick(const TArray<URoomArchetypeDA*>& Pool, FRandomStream& RNG, const TSet<FName>* ExcludeTags = nullptr) const;
    static void FillNodeFromArchetype(FRoomNode& OutNode, int32 Id, int32 Depth, const URoomArchetypeDA* A);
};

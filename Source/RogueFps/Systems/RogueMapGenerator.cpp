// RogueMapGenerator.cpp

#include "RogueMapGenerator.h"

DEFINE_LOG_CATEGORY(LogRogueMapGen);

URoomArchetypeDA* URogueMapGenerator::WeightedPick(
    const TArray<URoomArchetypeDA*>& Pool, FRandomStream& RNG, const TSet<FName>* ExcludeTags) const
{
    TArray<URoomArchetypeDA*> Candidates;
    Candidates.Reserve(Pool.Num());

    int32 TotalWeight = 0;
    for (URoomArchetypeDA* A : Pool)
    {
        if (!A) continue;
        if (ExcludeTags && ExcludeTags->Contains(A->Tag)) continue;
        const int32 W = FMath::Max<int32>(1, A->Weight);
        Candidates.Add(A);
        TotalWeight += W;
    }

    if (Candidates.Num() == 0 || TotalWeight <= 0) return nullptr;

    const int32 Pick = RNG.RandRange(1, TotalWeight);
    int32 Acc = 0;
    for (URoomArchetypeDA* A : Candidates)
    {
        const int32 W = FMath::Max<int32>(1, A->Weight);
        Acc += W;
        if (Pick <= Acc) return A;
    }
    return Candidates.Last();
}

void URogueMapGenerator::FillNodeFromArchetype(FRoomNode& Out, int32 Id, int32 Depth, const URoomArchetypeDA* A)
{
    Out.Id = Id; Out.Depth = Depth;
    Out.Type = A ? A->RoomType : ERoomType::Battle;
    Out.LevelRef = A ? A->LevelToStream : nullptr;
    Out.DangerTier = A ? A->DangerTier : 1;
    Out.Tag = A ? A->Tag : NAME_None;
}

FRogueLayerGraph URogueMapGenerator::GenerateLayer(const FRogueLayerConfig& Cfg, int32 Seed)
{
    FRandomStream RNG(Seed);
    FRogueLayerGraph G; G.SeedUsed = Seed;
    TSet<FName> UsedEventTags;
    int32 NodeId = 0;

    // Depth1: 固定普通战
    {
        URoomArchetypeDA* A = WeightedPick(Cfg.BattlePool, RNG);
        FRoomNode N; FillNodeFromArchetype(N, NodeId++, 1, A);
        if (N.Type == ERoomType::BattleHard) N.Type = ERoomType::Battle;
        FRoomColumn Col; Col.Nodes.Add(N);
        G.Columns.Add(1, Col);
    }

    // Depth2~6：二选一（战斗或事件）
    for (int32 Depth = 2; Depth <= 6; ++Depth)
    {
        // Battle / Hard
        bool bHard = (Cfg.BattleHardPool.Num() > 0) && (RNG.FRand() < Cfg.HardBattleChance);
        URoomArchetypeDA* BA = bHard ? WeightedPick(Cfg.BattleHardPool, RNG) : nullptr;
        if (!BA) BA = WeightedPick(Cfg.BattlePool, RNG);

        FRoomNode NB; FillNodeFromArchetype(NB, NodeId++, Depth, BA);
        NB.Type = (BA && BA->RoomType == ERoomType::BattleHard) ? ERoomType::BattleHard : ERoomType::Battle;

        // Event
        URoomArchetypeDA* EA = nullptr;
        if (Cfg.bUniqueEventsPerLayer)
        {
            EA = WeightedPick(Cfg.EventPool, RNG, &UsedEventTags);
            if (EA) UsedEventTags.Add(EA->Tag);
        }
        else EA = WeightedPick(Cfg.EventPool, RNG);

        FRoomNode NE; FillNodeFromArchetype(NE, NodeId++, Depth, EA);
        NE.Type = ERoomType::Event;

        FRoomColumn Col; Col.Nodes = { NB, NE }; // 约定 [0]=战斗，[1]=事件
        G.Columns.Add(Depth, Col);
    }

    // Depth7: Shop
    {
        URoomArchetypeDA* S = WeightedPick(Cfg.ShopPool, RNG);
        FRoomNode N; FillNodeFromArchetype(N, NodeId++, 7, S); N.Type = ERoomType::Shop;
        FRoomColumn Col; Col.Nodes.Add(N);
        G.Columns.Add(7, Col); G.ShopNodeId = N.Id;
    }

    // Depth8: Boss
    {
        URoomArchetypeDA* B = WeightedPick(Cfg.BossPool, RNG);
        FRoomNode N; FillNodeFromArchetype(N, NodeId++, 8, B); N.Type = ERoomType::Boss;
        FRoomColumn Col; Col.Nodes.Add(N);
        G.Columns.Add(8, Col); G.BossNodeId = N.Id;
    }

    return G;
}

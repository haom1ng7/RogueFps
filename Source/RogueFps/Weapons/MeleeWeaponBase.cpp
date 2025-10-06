#include "MeleeWeaponBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "RogueFps/Characters/RogueFpsCharacter.h"

// 你的自定义近战伤害类型（若还没建，可先注释本行并用 UDamageType）
//#include "DamageType_Melee.h"

AMeleeWeaponBase::AMeleeWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;

    DaggerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DaggerMesh"));
    SetRootComponent(DaggerMesh);

    // 近战不涉及弹药状态
    bIsReloading = false;
}

void AMeleeWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    if (ACharacter* Ch = Cast<ACharacter>(GetOwner()))
    {
        ArmMesh = Ch->GetMesh();
    }
}

void AMeleeWeaponBase::StartFire()
{
    // 1) 疾跑禁止（与远程一致）
    if (const APawn* P = Cast<APawn>(GetOwner()))
    {
        if (const ARogueFpsCharacter* C = Cast<ARogueFpsCharacter>(P))
        {
            if (C->IsSprinting()) return;
        }
    }

    // 2) 冷却
    const float Now = GetWorld()->TimeSeconds;
    if (Now - LastAttackTime < AttackCooldown) return;
    LastAttackTime = Now;

    DoViewSweepHit();

    // 3) 可选演出：播放轻击蒙太奇（不影响命中）
    if (ArmMesh && LightAttackMontage)
    {
        if (UAnimInstance* Inst = ArmMesh->GetAnimInstance())
        {
            Inst->Montage_Play(LightAttackMontage, 1.f);
        }
    }
}

void AMeleeWeaponBase::DoViewSweepHit()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    // 取视角起点/方向
    FVector EyeLoc; FRotator EyeRot;
    if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
        PC->GetPlayerViewPoint(EyeLoc, EyeRot);
    else
        OwnerPawn->GetActorEyesViewPoint(EyeLoc, EyeRot);

    const FVector Dir = EyeRot.Vector();
    const FVector Start = EyeLoc + Dir * ViewStartOffset;
    const FVector End = Start + Dir * ViewRange;

    // 忽略：自己/Owner/附着体/武器自身
    FCollisionQueryParams Params(SCENE_QUERY_STAT(Melee_ViewSweep), false, this);
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(OwnerPawn);
    if (AActor* OwnerActor = OwnerPawn)
    {
        TArray<AActor*> Attached; OwnerActor->GetAttachedActors(Attached);
        for (AActor* A : Attached) Params.AddIgnoredActor(A);
    }

    // 核心：按阵营选通道（和枪械一致的两个 Trace Channel）
    const bool bEnemyAttacker = OwnerPawn->ActorHasTag(TEXT("Team_Enemy"));
    const ECollisionChannel MeleeChannel = bEnemyAttacker
        ? ECC_GameTraceChannel2 /* WeaponTrace_Enemy */
        : ECC_GameTraceChannel1 /* WeaponTrace_Player */;

    TArray<FHitResult> Hits;
    GetWorld()->SweepMultiByChannel(
        Hits, Start, End, FQuat::Identity,
        MeleeChannel,                                   // 改这一个参数
        FCollisionShape::MakeSphere(ViewSphereRadius),
        Params
    );

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }

#if !(UE_BUILD_SHIPPING)
    DrawDebugCapsule(
        GetWorld(), (Start + End) * 0.5f, (End - Start).Size() * 0.5f, ViewSphereRadius,
        FRotationMatrix::MakeFromZ((End - Start).GetSafeNormal()).ToQuat(),
        Hits.Num() > 0 ? FColor::Red : FColor::Blue, false, 0.5f
    );
#endif

    ApplyMeleeHits(Hits, Dir);
}

void AMeleeWeaponBase::ApplyMeleeHits(const TArray<FHitResult>& Hits, const FVector& AttackDir)
{
    // TODO:近战伤害类型
    TSubclassOf<UDamageType> DamageClass = WeaponConfig->DamageType;

    bool bHitAny = false;

    for (const FHitResult& H : Hits)
    {
        AActor* HitActor = H.GetActor();
        if (!HitActor) continue;

        float FinalDamage = WeaponConfig->Damage;

        // 爆头倍率（沿用你已有的标签系统）
        if (UPrimitiveComponent* Comp = H.Component.Get())
        {
            if (HeadComponentTag != NAME_None && Comp->ComponentHasTag(HeadComponentTag) && WeaponConfig)
            {
                FinalDamage *= WeaponConfig->HeadshotMultiplier;
            }
        }

        UGameplayStatics::ApplyPointDamage(
            HitActor,
            FinalDamage,
            AttackDir.IsNearlyZero() ? (H.TraceEnd - H.TraceStart).GetSafeNormal() : AttackDir,
            H,
            GetInstigatorController(),
            this,
            DamageClass
        );

        bHitAny = true;
    }

    // 命中停顿（可选）
    if (bHitAny && HitStopTime > 0.f)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            PC->SetPause(true);
            FTimerHandle Th;
            GetWorldTimerManager().SetTimer(Th, [PC]()
                {
                    if (PC) PC->SetPause(false);
                }, HitStopTime, false);
        }
    }
}

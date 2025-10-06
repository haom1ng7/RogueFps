#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "MeleeWeaponBase.generated.h"

UCLASS()
class ROGUEFPS_API AMeleeWeaponBase : public AWeaponBase
{
    GENERATED_BODY()

public:
    AMeleeWeaponBase();

    // 按下即判定命中（瞬时）
    virtual void StartFire() override;

    // 近战不需要换弹
    virtual bool CanReload() const { return false; }
    virtual void Reload() override {}
    virtual void CancelReload() {}

protected:
    virtual void BeginPlay() override;

    /** 执行一次“相机前向球体扫掠”的近战命中（默认） */
    void DoViewSweepHit();

    /** 命中结算 */
    void ApplyMeleeHits(const TArray<FHitResult>& Hits, const FVector& AttackDir);

protected:
    /** 匕首网格（建议挂在角色全身 Mesh 的手部挂点） */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee|Mesh")
    USkeletalMeshComponent* DaggerMesh;

    /** 用于播放蒙太奇的手臂 Mesh（BeginPlay 自动从 Owner 的 GetMesh() 获取） */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Melee|Mesh")
    USkeletalMeshComponent* ArmMesh;

    /** 播放的轻击蒙太奇（仅演出，可为空） */
    UPROPERTY(EditDefaultsOnly, Category = "Melee|Anim")
    UAnimMontage* LightAttackMontage = nullptr;

    /** 刀刃 Socket 名（当 bUseBladeSegmentHit = true 时生效） */
    UPROPERTY(EditDefaultsOnly, Category = "Melee|Mode")
    FName BladeStartSocket = TEXT("Blade_Start");

    UPROPERTY(EditDefaultsOnly, Category = "Melee|Mode")
    FName BladeEndSocket = TEXT("Blade_End");

    /** —— 相机前向扫掠参数 —— */
    UPROPERTY(EditDefaultsOnly, Category = "Melee|ViewSweep")
    float ViewStartOffset = 50.f;

    UPROPERTY(EditDefaultsOnly, Category = "Melee|ViewSweep")
    float ViewRange = 160.f;

    UPROPERTY(EditDefaultsOnly, Category = "Melee|ViewSweep")
    float ViewSphereRadius = 28.f;

    /** —— 碰撞、伤害、节奏 —— */
    /** 近战命中使用的通道（建议 ECC_Pawn；若打不到可临时切 ECC_Visibility） */
    UPROPERTY(EditDefaultsOnly, Category = "Melee|Hit")
    TEnumAsByte<ECollisionChannel> MeleeTraceChannel = ECC_Pawn;

    /** 两次近战的最短间隔（秒） */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Melee|Hit")
    float AttackCooldown = 0.35f;

    /** （可选）命中时镜头停顿（秒），0 为关闭 */
    UPROPERTY(EditDefaultsOnly, Category = "Melee|FX")
    float HitStopTime = 0.f;

    /** 运行时缓存 */
    float LastAttackTime = -9999.f;
};

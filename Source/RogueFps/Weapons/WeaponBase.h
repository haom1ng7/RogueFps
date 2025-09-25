#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UWeaponConfigDA.h"
#include "WeaponBase.generated.h"

UCLASS()
class ROGUEFPS_API AWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AWeaponBase();

protected:
    virtual void BeginPlay() override;

    /** 配置数据 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    UWeaponConfigDA* WeaponConfig;

    /** 弹药状态 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    int32 CurrentAmmo;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    int32 CurrentReserve;

    /** 状态 */
    bool bIsReloading;
    float LastFireTime;

    /** 定时器 */
    FTimerHandle TimerHandle_AutoFire;
    FTimerHandle TimerHandle_Reload;

    /** 特效与音效 二选一*/
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    UParticleSystem* MuzzleFlash; // 旧 Cascade

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    class UNiagaraSystem* MuzzleNiagara; // 新 Niagara

    UPROPERTY(EditDefaultsOnly, Category = "SFX")
    USoundBase* FireSound;

    /** 枪口插槽名字 */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName MuzzleSocketName = "MuzzleFlashSocket";

    /** 弹壳特效 二选一 */
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    UParticleSystem* ShellEjectFX;

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    UNiagaraSystem* ShellEjectNiagara;

    /** 弹壳插槽名字 */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName ShellEjectSocketName = "EjectSocket";

    /** 后坐力参数 */
    UPROPERTY(EditDefaultsOnly, Category = "Recoil")
    float RecoilPitch = 1.0f; // 每发枪口上抬角度
    UPROPERTY(EditDefaultsOnly, Category = "Recoil")
    float RecoilYaw = 0.5f;   // 每发随机左右偏移

    /** 当前散布 */
    float CurrentSpread;

    // 开火动画（第一人称）
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* FireMontage;

    // 换弹动画（第一人称）
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* ReloadMontage;

public:
    /** 开火接口 */
    virtual void StartFire();
    virtual void StopFire();
    virtual void Fire();

    /** 换弹接口 */
    virtual void Reload();
    void FinishReload();

    /** 条件 */
    bool CanFire() const;
    bool CanReload() const;

    /** UI用 */
    FORCEINLINE int32 GetAmmo() const { return CurrentAmmo; }
    FORCEINLINE int32 GetReserve() const { return CurrentReserve; }
    FORCEINLINE bool IsReloading() const { return bIsReloading; }
};

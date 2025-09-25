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

    /** �������� */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    UWeaponConfigDA* WeaponConfig;

    /** ��ҩ״̬ */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    int32 CurrentAmmo;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    int32 CurrentReserve;

    /** ״̬ */
    bool bIsReloading;
    float LastFireTime;

    /** ��ʱ�� */
    FTimerHandle TimerHandle_AutoFire;
    FTimerHandle TimerHandle_Reload;

    /** ��Ч����Ч ��ѡһ*/
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    UParticleSystem* MuzzleFlash; // �� Cascade

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    class UNiagaraSystem* MuzzleNiagara; // �� Niagara

    UPROPERTY(EditDefaultsOnly, Category = "SFX")
    USoundBase* FireSound;

    /** ǹ�ڲ������ */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName MuzzleSocketName = "MuzzleFlashSocket";

    /** ������Ч ��ѡһ */
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    UParticleSystem* ShellEjectFX;

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    UNiagaraSystem* ShellEjectNiagara;

    /** ���ǲ������ */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName ShellEjectSocketName = "EjectSocket";

    /** ���������� */
    UPROPERTY(EditDefaultsOnly, Category = "Recoil")
    float RecoilPitch = 1.0f; // ÿ��ǹ����̧�Ƕ�
    UPROPERTY(EditDefaultsOnly, Category = "Recoil")
    float RecoilYaw = 0.5f;   // ÿ���������ƫ��

    /** ��ǰɢ�� */
    float CurrentSpread;

    // ���𶯻�����һ�˳ƣ�
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* FireMontage;

    // ������������һ�˳ƣ�
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* ReloadMontage;

public:
    /** ����ӿ� */
    virtual void StartFire();
    virtual void StopFire();
    virtual void Fire();

    /** �����ӿ� */
    virtual void Reload();
    void FinishReload();

    /** ���� */
    bool CanFire() const;
    bool CanReload() const;

    /** UI�� */
    FORCEINLINE int32 GetAmmo() const { return CurrentAmmo; }
    FORCEINLINE int32 GetReserve() const { return CurrentReserve; }
    FORCEINLINE bool IsReloading() const { return bIsReloading; }
};

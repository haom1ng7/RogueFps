#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UWeaponConfigDA.h"
#include "WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFinishReload, int32, CurrentAmmo, int32, CurrentReserve);

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

	bool ShouldAutoReload() const;   // 空仓且有备弹且允许自动

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

	// 判断爆头
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage|Headshot")
	FName HeadComponentTag = FName("HeadHit");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage|Headshot")
	FName BodyComponentTag = FName("BodyHit");

public:
	/** 开火接口 */
	virtual void StartFire();
	virtual void StopFire();
	virtual void Fire();

	/** 换弹接口 */
	virtual void Reload();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FinishReload();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void CancelReload();

	/** 子弹变化事件（用于 UI 刷新） */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FFinishReload ReloadFinished;

private:
	void BroadcastReloadFinished();

public:
	/** 条件 */
	bool CanFire() const;
	bool CanReload() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool bAutoReloadWhenEmpty = true;

	/** UI用 */
	FORCEINLINE int32 GetAmmo() const { return CurrentAmmo; }
	FORCEINLINE int32 GetReserve() const { return CurrentReserve; }
	FORCEINLINE bool IsReloading() const { return bIsReloading; }

	/** 是否启用按阵营使用自定义 Trace 通道（建议开启） */
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	bool bUseTeamTraceChannels = true;

	/** 玩家开火使用的 Trace 通道（在“项目设置-碰撞”里映射为 WeaponTrace_Player） */
	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (EditCondition = "bUseTeamTraceChannels"))
	TEnumAsByte<ECollisionChannel> PlayerTraceChannel = ECC_GameTraceChannel1;

	/** 敌人开火使用的 Trace 通道（在“项目设置-碰撞”里映射为 WeaponTrace_Enemy） */
	UPROPERTY(EditDefaultsOnly, Category = "Collision", meta = (EditCondition = "bUseTeamTraceChannels"))
	TEnumAsByte<ECollisionChannel> EnemyTraceChannel = ECC_GameTraceChannel2;
};

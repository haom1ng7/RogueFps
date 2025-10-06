#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShieldComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldChanged, float, NewShield, float, MaxShield);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldBroken, AActor*, DamagedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldRegenStateChanged, bool, bIsRegenerating);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class ROGUEFPS_API UShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShieldComponent();

	/** 最大护盾 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield")
	float MaxShield = 50.f;

	/** 护盾再生延迟（秒）——受击后等待多少秒再开始回盾 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Regen")
	float RegenDelay = 5.f;

	/** 护盾再生速度（每秒恢复多少） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Regen")
	float RegenRate = 15.f;

	/** 当前护盾（运行时变化） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield")
	float Shield = 0.f;

	/** 是否允许再生（有些状态下可临时禁用，例如冲刺、技能等） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield|Regen")
	bool bAllowRegen = true;

	/** 护盾变化事件（用于 UI 刷新） */
	UPROPERTY(BlueprintAssignable, Category = "Shield|Event")
	FOnShieldChanged OnShieldChanged;

	/** 破盾事件 */
	UPROPERTY(BlueprintAssignable, Category = "Shield|Event")
	FOnShieldBroken OnShieldBroken;

	/** 再生开始/停止事件（用于播放 VFX/SFX） */
	UPROPERTY(BlueprintAssignable, Category = "Shield|Event")
	FOnShieldRegenStateChanged OnRegenStateChanged;

	/** 由外部（例如 HealthComponent）调用，尝试用护盾吸收伤害，返回“被吸收的伤害量” */
	UFUNCTION(BlueprintCallable, Category = "Shield")
	float AbsorbDamage(float IncomingDamage, AController* Instigator, AActor* DamageCauser);

	/** 直接设置当前护盾（例如拾取道具） */
	UFUNCTION(BlueprintCallable, Category = "Shield")
	void SetShield(float NewValue);

	/** 当前护盾百分比（0~1） */
	UFUNCTION(BlueprintPure, Category = "Shield")
	float GetShieldPercent() const { return (MaxShield <= 0.f) ? 0.f : Shield / MaxShield; }

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle Timer_RegenDelay;
	FTimerHandle Timer_RegenTick;

	bool bIsRegenerating = false;

	void KickRegenDelay();
	void StartRegen();
	void StopRegen();
	void RegenTick(); // 每帧/定时恢复
	void BroadcastShieldChanged();
};

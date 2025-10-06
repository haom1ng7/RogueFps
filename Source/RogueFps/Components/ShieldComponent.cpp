#include "ShieldComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UShieldComponent::UShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // 用定时器即可
}

void UShieldComponent::BeginPlay()
{
	Super::BeginPlay();
	Shield = FMath::Clamp(MaxShield, 0.f, MaxShield);
	BroadcastShieldChanged();
}

void UShieldComponent::BroadcastShieldChanged()
{
	OnShieldChanged.Broadcast(Shield, MaxShield);
}

void UShieldComponent::KickRegenDelay()
{
	// 受击后：停止正在进行的再生，重置延迟计时
	StopRegen();

	if (!bAllowRegen || RegenDelay <= 0.f) return;

	// 延迟到时开始再生
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(Timer_RegenDelay);
		W->GetTimerManager().SetTimer(
			Timer_RegenDelay,
			this, &UShieldComponent::StartRegen,
			RegenDelay, false
		);
	}
}

void UShieldComponent::StartRegen()
{
	if (!bAllowRegen || Shield >= MaxShield || RegenRate <= 0.f) return;

	if (UWorld* W = GetWorld())
	{
		bIsRegenerating = true;
		OnRegenStateChanged.Broadcast(true);

		// 用较小 Tick 周期获得平滑曲线
		const float Period = 0.1f;
		W->GetTimerManager().SetTimer(
			Timer_RegenTick,
			this, &UShieldComponent::RegenTick,
			Period, true
		);
	}
}

void UShieldComponent::StopRegen()
{
	if (!bIsRegenerating) return;

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(Timer_RegenTick);
	}
	bIsRegenerating = false;
	OnRegenStateChanged.Broadcast(false);
}

void UShieldComponent::RegenTick()
{
	if (!bAllowRegen || RegenRate <= 0.f)
	{
		StopRegen();
		return;
	}

	// Δt 用定时器周期（0.1s）
	const float Delta = 0.1f * RegenRate;
	const float Old = Shield;
	Shield = FMath::Min(Shield + Delta, MaxShield);

	if (!FMath::IsNearlyEqual(Old, Shield))
	{
		BroadcastShieldChanged();
	}

	if (Shield >= MaxShield)
	{
		StopRegen();
	}
}

void UShieldComponent::SetShield(float NewValue)
{
	const float Clamped = FMath::Clamp(NewValue, 0.f, MaxShield);
	const bool bWasBroken = (Shield <= 0.f);

	Shield = Clamped;
	BroadcastShieldChanged();

	// 如果从 0 回到 >0，可以选择在这里触发某些效果
	if (bWasBroken && Shield > 0.f)
	{
		// e.g. Play shield-restore VFX
	}
}

float UShieldComponent::AbsorbDamage(float IncomingDamage, AController* Instigator, AActor* DamageCauser)
{
	if (IncomingDamage <= 0.f || MaxShield <= 0.f) return 0.f;

	// 一旦受击，打断/重置回盾计时
	KickRegenDelay();

	if (Shield <= 0.f)
	{
		return 0.f; // 已无盾可吸收
	}

	const float Absorbed = FMath::Min(Shield, IncomingDamage);
	const float Old = Shield;

	Shield -= Absorbed;
	BroadcastShieldChanged();

	if (Old > 0.f && Shield <= 0.f)
	{
		// 破盾
		OnShieldBroken.Broadcast(GetOwner());
	}

	return Absorbed;
}

#include "StaminaComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsSprinting) { DrainForSprint(DeltaTime); }
	else if (bAllowRegen) { HandleRegen(DeltaTime); }
}

bool UStaminaComponent::TryStartSprint()
{
	if (!CanSprint()) return false;
	bAllowRegen = false;
	StartRegenTimer(); // 停止后才恢复
	return true;
}

void UStaminaComponent::StopSprint()
{
	bIsSprinting = false;
	StartRegenTimer();
}

bool UStaminaComponent::TryDash()
{
	if (!CanDash()) return false;
	bAllowRegen = false;
	ApplyStamina(Stamina - DashCost);
	StartRegenTimer();
	return true;
}

//bool UStaminaComponent::TryJump()
//{
//	if (!CanJump()) return false;
//	bAllowRegen = false;
//	StartRegenTimer(); // 停止后才恢复
//	ApplyStamina(Stamina - JumpCost);
//	return true;
//}

void UStaminaComponent::AddStamina(float Delta)
{
	ApplyStamina(Stamina + Delta, /*bBroadcastDepleted*/false);
}

void UStaminaComponent::DrainForSprint(float DeltaTime)
{
	if (SprintDrainRate <= 0.f) return;
	ApplyStamina(Stamina - SprintDrainRate * DeltaTime);

	// 见底自动停
	if (Stamina <= 0.f)
	{
		bIsSprinting = false;
		StartRegenTimer();
	}
}

void UStaminaComponent::HandleRegen(float DeltaTime)
{
	if (RegenRate <= 0.f || Stamina >= MaxStamina) return;
	ApplyStamina(Stamina + RegenRate * DeltaTime, /*bBroadcastDepleted*/false);
}

void UStaminaComponent::ApplyStamina(float NewValue, bool bBroadcastDepleted)
{
	const float Old = Stamina;
	Stamina = FMath::Clamp(NewValue, 0.f, MaxStamina);
	if (!FMath::IsNearlyEqual(Old, Stamina))
	{
		const bool bDepleted = bBroadcastDepleted && (Stamina <= 0.f);
		OnStaminaChanged.Broadcast(Stamina, MaxStamina, bDepleted);
	}
}

void UStaminaComponent::StartRegenTimer()
{
	bAllowRegen = false;
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(RegenTimerHandle);
		W->GetTimerManager().SetTimer(
			RegenTimerHandle,
			[this]() { bAllowRegen = true; },
			FMath::Max(0.f, RegenDelay),
			false
		);
	}
}

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStaminaChanged, float, NewStamina, float, MaxStamina, bool, bDepleted);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ROGUEFPS_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UStaminaComponent();

	// 可调参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stamina")
	float Stamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Drain")
	float SprintDrainRate = 24.f;        // 疾跑每秒消耗

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Drain")
	float DashCost = 35.f;               // Dash 一次性消耗

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Drain")
	float JumpCost = 35.f;               // Jump 一次性消耗

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Regen")
	float RegenRate = 10.f;              // 每秒恢复

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Regen")
	float RegenDelay = 1.2f;             // 停止消耗后多久开始恢复

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Tuning")
	float LowThreshold = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|State")
	bool bIsSprinting = false;

	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaChanged OnStaminaChanged;

public:
	// 外部接口
	UFUNCTION(BlueprintCallable) bool TryStartSprint();
	UFUNCTION(BlueprintCallable) void  StopSprint();
	UFUNCTION(BlueprintCallable) bool TryDash();
	//UFUNCTION(BlueprintCallable) bool TryJump();
	UFUNCTION(BlueprintCallable) void  AddStamina(float Delta);

	UFUNCTION(BlueprintPure) bool CanSprint() const { return Stamina > KINDA_SMALL_NUMBER; }
	UFUNCTION(BlueprintPure) bool CanDash()   const { return Stamina >= DashCost; }
	//UFUNCTION(BlueprintPure) bool CanJump()   const { return Stamina >= JumpCost; }
	UFUNCTION(BlueprintPure) float GetStaminaPercent() const { return MaxStamina <= 0 ? 0.f : Stamina / MaxStamina; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void DrainForSprint(float DeltaTime);
	void HandleRegen(float DeltaTime);
	void ApplyStamina(float NewValue, bool bBroadcastDepleted = true);
	void StartRegenTimer();

	FTimerHandle RegenTimerHandle;
	bool bAllowRegen = true;
};

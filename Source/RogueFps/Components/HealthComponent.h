#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// 生命值变化事件
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(
    FOnHealthChangedSignature,
    UHealthComponent*, HealthComp,   // 哪个组件
    float, Health,                   // 当前生命值
    float, HealthDelta,              // 本次变化值 (+回血 / -掉血)
    const class UDamageType*, DamageType,
    class AController*, InstigatedBy,
    AActor*, DamageCauser
);

// 死亡事件
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ROGUEFPS_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

protected:
    virtual void BeginPlay() override;

    // 最大血量
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
    float MaxHealth = 100.0f;

    // 当前生命值
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    float Health;

    // 接收伤害回调
    UFUNCTION()
    void HandleTakeAnyDamage(
        AActor* DamagedActor,
        float Damage,
        const class UDamageType* DamageType,
        class AController* InstigatedBy,
        AActor* DamageCauser
    );

public:
    // 生命值变化事件
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthChangedSignature OnHealthChanged;

    // 死亡事件
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDeathSignature OnDeath;

    // 获取当前生命值
    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetHealth() const { return Health; }

    // 治疗函数
    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    // 主动造成伤害
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamage(float DamageAmount);
};

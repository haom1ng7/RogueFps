#include "HealthComponent.h"
#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    Health = MaxHealth; // 开始时 = 最大血量

    AActor* MyOwner = GetOwner();
    if (MyOwner)
    {
        // 绑定 UE 自带的伤害事件
        MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
    }

    // 初始化血量
    OnHealthChanged.Broadcast(this, Health, 0.0f, nullptr, nullptr, nullptr);
}

void UHealthComponent::HandleTakeAnyDamage(
    AActor* DamagedActor,
    float Damage,
    const class UDamageType* DamageType,
    class AController* InstigatedBy,
    AActor* DamageCauser
)
{
    if (Damage <= 0.0f || Health <= 0.0f)
    {
        return;
    }

    float OldHealth = Health;

    // 扣血
    Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

    float ActualDamage = OldHealth - Health; // 计算实际伤害 防止伤害溢出

    // 触发事件
    OnHealthChanged.Broadcast(this, Health, -ActualDamage, DamageType, InstigatedBy, DamageCauser);
    if (Health <= 0.0f)
    {
        OnDeath.Broadcast();
    }
}

void UHealthComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f || Health <= 0.0f)
    {
        return;
    }

    float OldHealth = Health;

    // 回血
    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);

    float ActualHeal = Health - OldHealth;

    // 触发事件
    OnHealthChanged.Broadcast(this, Health, ActualHeal, nullptr, nullptr, nullptr);
}

// 掉血函数
void UHealthComponent::ApplyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f || Health <= 0.0f)
    {
        return;
    }

    float OldHealth = Health;

    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);

    float ActualDamage = OldHealth - Health;

    OnHealthChanged.Broadcast(this, Health, -ActualDamage, nullptr, nullptr, nullptr);
    if (Health <= 0.0f)
    {
        OnDeath.Broadcast();
    }
}

#include "HealthComponent.h"
#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    Health = MaxHealth; // ��ʼʱ = ���Ѫ��

    AActor* MyOwner = GetOwner();
    if (MyOwner)
    {
        // �� UE �Դ����˺��¼�
        MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
    }

    // ��ʼ��Ѫ��
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

    // ��Ѫ
    Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

    float ActualDamage = OldHealth - Health; // ����ʵ���˺� ��ֹ�˺����

    // �����¼�
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

    // ��Ѫ
    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);

    float ActualHeal = Health - OldHealth;

    // �����¼�
    OnHealthChanged.Broadcast(this, Health, ActualHeal, nullptr, nullptr, nullptr);
}

// ��Ѫ����
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

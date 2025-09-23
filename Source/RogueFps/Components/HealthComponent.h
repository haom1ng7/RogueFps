#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// ����ֵ�仯�¼�
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(
    FOnHealthChangedSignature,
    UHealthComponent*, HealthComp,   // �ĸ����
    float, Health,                   // ��ǰ����ֵ
    float, HealthDelta,              // ���α仯ֵ (+��Ѫ / -��Ѫ)
    const class UDamageType*, DamageType,
    class AController*, InstigatedBy,
    AActor*, DamageCauser
);

// �����¼�
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ROGUEFPS_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

protected:
    virtual void BeginPlay() override;

    // ���Ѫ��
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
    float MaxHealth = 100.0f;

    // ��ǰ����ֵ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    float Health;

    // �����˺��ص�
    UFUNCTION()
    void HandleTakeAnyDamage(
        AActor* DamagedActor,
        float Damage,
        const class UDamageType* DamageType,
        class AController* InstigatedBy,
        AActor* DamageCauser
    );

public:
    // ����ֵ�仯�¼�
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthChangedSignature OnHealthChanged;

    // �����¼�
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDeathSignature OnDeath;

    // ��ȡ��ǰ����ֵ
    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetHealth() const { return Health; }

    // ���ƺ���
    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    // ��������˺�
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamage(float DamageAmount);
};

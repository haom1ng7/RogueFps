#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/WeaponBase.h"
#include "WeaponComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ROGUEFPS_API UWeaponComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWeaponComponent();

protected:
    virtual void BeginPlay() override;

    /** ��ʼ�����������ö��֣� */
    UPROPERTY(EditAnywhere, Category = "Weapon")
    TArray<TSubclassOf<AWeaponBase>> StarterWeapons;

    /** ��ǰװ�������� */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    AWeaponBase* CurrentWeapon;

    /** ��ǰ���е������б� */
    UPROPERTY()
    TArray<AWeaponBase*> Weapons;

    /** �����ҵ� */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponAttachSocketName = "WeaponSocket";

public:
    /** �������� */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void StartFire();
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void StopFire();
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Reload();

    /** �л����� */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void SwitchWeapon(int32 Index);

    /** ��ȡ��ǰ���� */
    FORCEINLINE AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
};

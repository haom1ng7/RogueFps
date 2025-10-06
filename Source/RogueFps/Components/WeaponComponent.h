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

    /** 初始武器（可设置多种） */
    UPROPERTY(EditAnywhere, Category = "Weapon")
    TArray<TSubclassOf<AWeaponBase>> StarterWeapons;

    /** 当前装备的武器 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    AWeaponBase* CurrentWeapon;

    /** 当前持有的武器列表 */
    UPROPERTY()
    TArray<AWeaponBase*> Weapons;

    /** 武器挂点 */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponAttachSocketName = "WeaponSocket";

public:
    /** 武器操作 */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void StartFire();
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void StopFire();
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Reload();

    /** 切换武器 */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void SwitchWeapon(int32 Index);

    /** 获取当前武器 */
    FORCEINLINE AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
};

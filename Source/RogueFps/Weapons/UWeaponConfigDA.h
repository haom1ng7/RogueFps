// WeaponConfigDA.h
#pragma once

#include "Engine/DataAsset.h"
#include "UWeaponConfigDA.generated.h"

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
    Single UMETA(DisplayName = "Single"),
    Auto   UMETA(DisplayName = "Auto")
};

UCLASS(BlueprintType)
class ROGUEFPS_API UWeaponConfigDA : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    EWeaponFireMode FireMode = EWeaponFireMode::Single;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    float Damage = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    float FireRate = 600.f; // RPM

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    int32 MagSize = 12;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    int32 MaxReserve = 120;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    float ReloadTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    TSubclassOf<UDamageType> DamageType;

    /** 基础散布角度（度数） */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Spread")
    float BaseSpread = 1.0f;

    /** 连发额外散布（每次开火叠加） */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Spread")
    float RecoilSpread = 0.2f;

    /** 最大散布限制 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Spread")
    float MaxSpread = 5.0f;

};

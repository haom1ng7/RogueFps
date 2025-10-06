#include "WeaponComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

UWeaponComponent::UWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::BeginPlay()
{
    Super::BeginPlay();

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Éú³É StarterWeapons
    for (auto& WeaponClass : StarterWeapons)
    {
        if (WeaponClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            SpawnParams.Owner = OwnerChar;

            AWeaponBase* NewWeapon = World->SpawnActor<AWeaponBase>(WeaponClass, SpawnParams);
            if (NewWeapon)
            {
                NewWeapon->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
                Weapons.Add(NewWeapon);
            }
        }
    }

    if (Weapons.Num() > 0)
    {
        CurrentWeapon = Weapons[0];
    }
}

void UWeaponComponent::StartFire()
{
    if (CurrentWeapon) CurrentWeapon->StartFire();
}

void UWeaponComponent::StopFire()
{
    if (CurrentWeapon) CurrentWeapon->StopFire();
}

void UWeaponComponent::Reload()
{
    if (CurrentWeapon) CurrentWeapon->Reload();
}

void UWeaponComponent::SwitchWeapon(int32 Index)
{
    if (Weapons.IsValidIndex(Index))
    {
        CurrentWeapon = Weapons[Index];
    }
}

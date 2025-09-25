#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Animation/AnimInstance.h"

AWeaponBase::AWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;
    bIsReloading = false;
    LastFireTime = -9999.f;
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    if (WeaponConfig)
    {
        CurrentAmmo = WeaponConfig->MagSize;
        CurrentReserve = WeaponConfig->MaxReserve;
    }
}

bool AWeaponBase::CanFire() const
{
    if (!WeaponConfig || bIsReloading || CurrentAmmo <= 0) return false;
    float TimeBetweenShots = 60.f / WeaponConfig->FireRate;
    return (GetWorld()->TimeSeconds - LastFireTime) >= TimeBetweenShots;
}

void AWeaponBase::StartFire()
{
    if (!WeaponConfig) return;

    if (WeaponConfig->FireMode == EWeaponFireMode::Single)
    {
        Fire();
    }
    else if (WeaponConfig->FireMode == EWeaponFireMode::Auto)
    {
        Fire();
        float TimeBetweenShots = 60.f / WeaponConfig->FireRate;
        GetWorldTimerManager().SetTimer(TimerHandle_AutoFire, this, &AWeaponBase::Fire, TimeBetweenShots, true);
    }
}

// 全自动武器在结束开火需调用
void AWeaponBase::StopFire()
{
    if (WeaponConfig && WeaponConfig->FireMode == EWeaponFireMode::Auto)
    {
        GetWorldTimerManager().ClearTimer(TimerHandle_AutoFire);
    }
}

void AWeaponBase::Fire()
{
    if (!CanFire()) return;

    CurrentAmmo--;
    LastFireTime = GetWorld()->TimeSeconds;

    // =======================
    // 1. 计算射击方向（加入散布）
    // =======================
    FVector EyeLoc;
    FRotator EyeRot;
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
        {
            PC->GetPlayerViewPoint(EyeLoc, EyeRot); // 从相机拿视角
        }
        else
        {
            // 备用：从 Pawn 的眼睛拿视角（AI 可能会用到）
            OwnerPawn->GetActorEyesViewPoint(EyeLoc, EyeRot);
        }
    }

    FVector ShotDir = EyeRot.Vector();

    // 随机散布角度
    float SpreadRad = FMath::DegreesToRadians(CurrentSpread);
    FVector ShootCone = FMath::VRandCone(ShotDir, SpreadRad);

    FVector TraceEnd = EyeLoc + (ShootCone * 10000.f);

    // =======================
    // 2. 命中检测
    // =======================
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLoc, TraceEnd, ECC_Visibility, Params))
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor)
        {
            // 走ue内设伤害
            UGameplayStatics::ApplyPointDamage(
                HitActor,
                WeaponConfig->Damage,
                ShootCone, // 用散布后的方向
                Hit,
                GetInstigatorController(),
                this,
                WeaponConfig->DamageType
            );
        }
        UE_LOG(LogTemp, Warning, TEXT("ShotDir = %s"), *ShotDir.ToString());
        DrawDebugLine(GetWorld(), EyeLoc, Hit.Location, FColor::Red, false, 1.0f, 0, 1.0f); // 红线命中
    }
    else
    {
        DrawDebugLine(GetWorld(), EyeLoc, TraceEnd, FColor::Blue, false, 1.0f, 0, 1.0f); // 蓝线未命中
    }

    // =======================
    // 3. 散布叠加
    // =======================
    if (WeaponConfig)
    {
        CurrentSpread = FMath::Clamp(CurrentSpread + WeaponConfig->RecoilSpread, WeaponConfig->BaseSpread, WeaponConfig->MaxSpread);
    }

    // =======================
    // 4. TODO：枪口火焰特效
    // =======================
    USceneComponent* RootComp = GetRootComponent();
    if (RootComp)
    {
        // 旧Cascade
        if (MuzzleFlash)
        {
            UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, RootComp, MuzzleSocketName);
        }
        // 新Niagara
        if (MuzzleNiagara)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleNiagara, RootComp, MuzzleSocketName,
                FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
        }
    }

    // =======================
    // 5. TODO：弹壳抛出
    // =======================
    if (RootComp)
    {
        if (ShellEjectFX)
        {
            UGameplayStatics::SpawnEmitterAttached(ShellEjectFX, RootComp, ShellEjectSocketName);
        }
        if (ShellEjectNiagara)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(ShellEjectNiagara, RootComp, ShellEjectSocketName,
                FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
        }
    }

    // =======================
    // 6. 枪声
    // =======================
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }

    // =======================
    // 7. TODO：动画 
    // =======================
    if (OwnerPawn)
    {
        // 找到 Arms Mesh（第一人称手臂）
        if (USkeletalMeshComponent* ArmsMesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>())
        {
            if (UAnimInstance* AnimInst = ArmsMesh->GetAnimInstance())
            {
                if (FireMontage)
                {
                    // 播放开火蒙太奇
                    AnimInst->Montage_Play(FireMontage);
                }
            }
        }

        // =======================
        // 8. 后坐力
        // =======================
        if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
        {
            float RandomYaw = FMath::RandRange(-RecoilYaw, RecoilYaw);
            PC->AddPitchInput(-RecoilPitch); // 镜头上抬
            PC->AddYawInput(RandomYaw);      // 左右偏移
        }
    }
}


bool AWeaponBase::CanReload() const
{
    return (!bIsReloading && CurrentAmmo < WeaponConfig->MagSize && CurrentReserve > 0);
}

void AWeaponBase::Reload()
{
    if (!CanReload()) return;

    bIsReloading = true;
    GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &AWeaponBase::FinishReload, WeaponConfig->ReloadTime, false);
}

void AWeaponBase::FinishReload()
{
    int32 AmmoNeeded = WeaponConfig->MagSize - CurrentAmmo;
    int32 AmmoToReload = FMath::Min(AmmoNeeded, CurrentReserve);

    CurrentAmmo += AmmoToReload;
    CurrentReserve -= AmmoToReload;

    bIsReloading = false;
}

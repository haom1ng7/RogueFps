#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "RogueFps/Characters/RogueFpsCharacter.h"

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
	// 疾跑期间禁止开火
	if (const APawn* P = Cast<APawn>(GetOwner()))
	{
		if (const ARogueFpsCharacter* C = Cast<ARogueFpsCharacter>(P))
		{
			if (C->IsSprinting()) return false;
		}
	}
	float TimeBetweenShots = 60.f / WeaponConfig->FireRate;
	return (GetWorld()->TimeSeconds - LastFireTime) >= TimeBetweenShots;
}

bool AWeaponBase::ShouldAutoReload() const
{
	return bAutoReloadWhenEmpty && (CurrentAmmo == 0) && (CurrentReserve > 0) && !bIsReloading;
}

void AWeaponBase::StartFire()
{
	if (!WeaponConfig) return;

	if (!CanFire())
	{
		if (ShouldAutoReload()) { Reload(); }
		return;
	}

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

	// 开火后打空：立刻进入自动换弹（不需要再次点击）
	if (ShouldAutoReload())
	{
		Reload();
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
			OwnerPawn->GetActorEyesViewPoint(EyeLoc, EyeRot);
		}
	}

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	FVector AimAt = FVector::ZeroVector;

	if (PlayerChar && PlayerChar != GetOwner()) // 避免自己是玩家时瞄自己
	{
		AimAt = PlayerChar->GetActorLocation();

		if (USkeletalMeshComponent* Mesh = PlayerChar->GetMesh())
		{
			const UCapsuleComponent* Cap = PlayerChar->GetCapsuleComponent();
			AimAt = PlayerChar->GetActorLocation() + FVector(0, 0, Cap->GetScaledCapsuleHalfHeight()); // 近似头/胸
		}
	}
	// 计算发射方向（带pitch），再叠加散布
	FVector ShotDir = AimAt.IsNearlyZero() ? EyeRot.Vector()
		: (AimAt - EyeLoc).GetSafeNormal();
	const float SpreadRad = FMath::DegreesToRadians(CurrentSpread);
	const FVector ShootDir = FMath::VRandCone(ShotDir, SpreadRad);
	const FVector TraceEnd = EyeLoc + ShootDir * 10000.f;

	// =======================
	// 2. 命中检测（使用阵营 Trace 通道）
	// =======================
	// 关键：根据开火者阵营选择 Trace 通道
	ECollisionChannel FireChannel = ECC_Visibility; // 兜底（如果还没配置通道）
	if (OwnerPawn)
	{
		const bool bEnemyShooter = OwnerPawn->ActorHasTag(TEXT("Team_Enemy"));
		FireChannel = bEnemyShooter ? ECC_GameTraceChannel2 /*WeaponTrace_Enemy*/
			: ECC_GameTraceChannel1 /*WeaponTrace_Player*/;
	}

	// 防止命中自身
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), /*TraceComplex*/ true);
	Params.AddIgnoredActor(this);
	if (AActor* OwnerActor = GetOwner())
	{
		Params.AddIgnoredActor(OwnerActor);
		TArray<AActor*> Attached;
		OwnerActor->GetAttachedActors(Attached);
		Params.AddIgnoredActors(Attached);
	}

	// 用 MultiTrace，拿第一个有效命中
	TArray<FHitResult> Hits;
	FHitResult FinalHit;
	AActor* FinalHitActor = nullptr;

	if (GetWorld()->LineTraceMultiByChannel(Hits, EyeLoc, TraceEnd, FireChannel, Params))
	{
		for (const FHitResult& H : Hits)
		{
			AActor* HitActor = H.GetActor();
			if (!HitActor) continue;

			// 通道已保证不会命中友军，这里直接接收第一个命中
			FinalHit = H;
			FinalHitActor = HitActor;
			break;
		}
	}

	if (FinalHitActor)
	{
		float FinalDamage = WeaponConfig ? WeaponConfig->Damage : 0.f;
		bool bHeadshot = false;

		if (UPrimitiveComponent* HitComp = FinalHit.Component.Get())
		{
			if (HeadComponentTag != NAME_None && HitComp->ComponentHasTag(HeadComponentTag))
			{
				bHeadshot = true;
			}
			else if (BodyComponentTag != NAME_None && HitComp->ComponentHasTag(BodyComponentTag))
			{
				bHeadshot = false;
			}
		}
		if (bHeadshot && WeaponConfig)
		{
			FinalDamage *= WeaponConfig->HeadshotMultiplier;
		}

		DrawDebugLine(GetWorld(), EyeLoc, FinalHit.Location, FColor::Red, false, 1.0f, 0, 1.0f);

		TSubclassOf<UDamageType> DamageTypeClass =
			(WeaponConfig && WeaponConfig->DamageType)
			? WeaponConfig->DamageType
			: TSubclassOf<UDamageType>(UDamageType::StaticClass());

		UGameplayStatics::ApplyPointDamage(
			FinalHitActor,
			FinalDamage,
			ShootDir,
			FinalHit,
			OwnerPawn ? OwnerPawn->GetController() : nullptr,
			this,
			DamageTypeClass
		);
	}
	else
	{
		DrawDebugLine(GetWorld(), EyeLoc, TraceEnd, FColor::Blue, false, 1.0f, 0, 1.0f);
	}

	// =======================
	// 3. 散布叠加
	// =======================
	if (WeaponConfig)
	{
		CurrentSpread = FMath::Clamp(
			CurrentSpread + WeaponConfig->RecoilSpread,
			WeaponConfig->BaseSpread,
			WeaponConfig->MaxSpread
		);
	}

	// =======================
	// 4. 枪口火焰特效
	// =======================
	USceneComponent* RootComp = GetRootComponent();
	if (RootComp)
	{
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, RootComp, MuzzleSocketName);
		}
		if (MuzzleNiagara)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				MuzzleNiagara, RootComp, MuzzleSocketName,
				FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true
			);
		}
	}

	// =======================
	// 5. 弹壳抛出
	// =======================
	if (RootComp)
	{
		if (ShellEjectFX)
		{
			UGameplayStatics::SpawnEmitterAttached(ShellEjectFX, RootComp, ShellEjectSocketName);
		}
		if (ShellEjectNiagara)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				ShellEjectNiagara, RootComp, ShellEjectSocketName,
				FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true
			);
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
	// 7. 动画 & 8. 后坐力
	// =======================
	if (OwnerPawn)
	{
		if (USkeletalMeshComponent* ArmsMesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (UAnimInstance* AnimInst = ArmsMesh->GetAnimInstance())
			{
				if (FireMontage)
				{
					AnimInst->Montage_Play(FireMontage);
				}
			}
		}

		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			const float RandomYaw = FMath::RandRange(-RecoilYaw, RecoilYaw);
			PC->AddPitchInput(-RecoilPitch);
			PC->AddYawInput(RandomYaw);
		}
	}

}



bool AWeaponBase::CanReload() const
{
	// 疾跑期间禁止换弹
	if (const APawn* P = Cast<APawn>(GetOwner()))
	{
		if (const ARogueFpsCharacter* C = Cast<ARogueFpsCharacter>(P))
		{
			if (C->IsSprinting()) return false;
		}
	}
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
	BroadcastReloadFinished();
}

void AWeaponBase::CancelReload()
{
	if (bIsReloading)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Reload);
		bIsReloading = false;

		UE_LOG(LogTemp, Warning, TEXT("Reload canceled"));
	}
}

void AWeaponBase::BroadcastReloadFinished()
{
	ReloadFinished.Broadcast(CurrentAmmo, CurrentReserve);
}

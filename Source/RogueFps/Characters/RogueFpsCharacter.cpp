// Copyright Epic Games, Inc. All Rights Reserved.

#include "RogueFpsCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RogueFps.h"
#include "Weapons/WeaponBase.h"

ARogueFpsCharacter::ARogueFpsCharacter()
{
	// 启用 visibility 碰撞检测
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 初始化 dash 布尔变量
	bIsDashing = false;

	// bind the attack montage ended delegate
	OnDashMontageEnded.BindUObject(this, &ARogueFpsCharacter::DashMontageEnded);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ARogueFpsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARogueFpsCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ARogueFpsCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARogueFpsCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARogueFpsCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ARogueFpsCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogRogueFps, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ARogueFpsCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ARogueFpsCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ARogueFpsCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ARogueFpsCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);

		// 没有前进输入时自动退出疾跑，防止原地“疾跑”
		if (bIsSprinting && FMath::IsNearlyZero(Forward, 0.05f))
		{
			StopSprint();
		}
	}
}


void ARogueFpsCharacter::DoDash()
{
	//TODO ： 退出瞄准

	// 避免疾跑与dash冲突
	if (bIsSprinting) StopSprint();
	// ignore the input if we've already dashed and have yet to reset
	if (bIsDashing)
		return;

	// raise the dash flags
	bIsDashing = true;

	// disable gravity while dashing
	GetCharacterMovement()->GravityScale = 0.0f;

	// reset the character velocity so we don't carry momentum into the dash
	GetCharacterMovement()->Velocity = FVector::ZeroVector;

	// play the dash montage
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(DashMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

		// has the montage played successfully?
		if (MontageLength > 0.0f)
		{
			AnimInstance->Montage_SetEndDelegate(OnDashMontageEnded, DashMontage);
		}
	}
	// restore gravity
	GetCharacterMovement()->GravityScale = 1.0f;
}

void ARogueFpsCharacter::DashMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// if the montage was interrupted, end the dash
	EndDash();
}

void ARogueFpsCharacter::EndDash()
{
	// reset the dashing flag
	bIsDashing = false;
}

void ARogueFpsCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ARogueFpsCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void ARogueFpsCharacter::Dash()
{
	// route the input
	DoDash();
}

void ARogueFpsCharacter::ApplySprint(bool bEnable)
{
	// 冲刺(Dash)中、或没前进输入时，不允许进入疾跑
	if (bEnable)
	{
		const FVector Vel2D = GetVelocity();
		const float Forwardness = FVector::DotProduct(Vel2D.GetSafeNormal2D(), GetActorForwardVector());
		if (bIsDashing || Forwardness <= 0.2f)
		{
			bEnable = false;
		}
	}

	bIsSprinting = bEnable;
	GetCharacterMovement()->MaxWalkSpeed = bEnable ? SprintSpeed : WalkSpeed;
}

void ARogueFpsCharacter::StartSprint()
{
	// TODO : 退出瞄准
	if (bIsDashing) return;

	ApplySprint(true);
}

void ARogueFpsCharacter::StopSprint()
{
	ApplySprint(false);
}

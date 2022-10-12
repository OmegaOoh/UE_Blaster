// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}



void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	initialBaseSpeed = BaseSpeed;
	initialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	initialJumpVelocity = Velocity;
}


void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

/*
* Heal Buff
*/

void UBuffComponent::Heal(float Amount, float Time)
{
	bHealing = true;
	HealingRate = Amount / Time;
	AmountToHeal += Amount;
}


void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;
	if(AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

/*
 * Shield Buff
 */

void UBuffComponent::ReplenishShield(float Amount, float Time)
{
	bReplenishing = true;
	ReplenishRate = Amount / Time;
	AmountToReplenish += Amount;
}


void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishing || Character == nullptr || Character->IsElimmed()) return;

	const float ReplenishThisFrame = ReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	AmountToReplenish -= ReplenishThisFrame;
	if (AmountToReplenish <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishing = false;
		AmountToReplenish = 0.f;
	}
}

/*
 * Speed Buff
 */

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float Time)
{
	if(Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeed,
		Time
		);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed,BuffCrouchSpeed);
}
void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr) return;

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = initialBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = initialCrouchSpeed;
	}
	MulticastSpeedBuff(initialBaseSpeed, initialCrouchSpeed);

}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

/*
 * JumpBuff
 */


void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::ResetJump()
{
	Character->GetCharacterMovement()->JumpZVelocity = initialJumpVelocity;
	MulticastJumpBuff(initialJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if(Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}


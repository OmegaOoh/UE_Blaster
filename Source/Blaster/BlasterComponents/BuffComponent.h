// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend  class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	//Buff Function
	void Heal(float Amount, float Time);
	void ReplenishShield(float Amount, float Time);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float Time);
	//Set Initial Value
	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);

protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);

private:
	UPROPERTY()
	ABlasterCharacter* Character;
	/*
	 * Heal Buff
	 */
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	/*
	 * Shield Buff
	 */

	bool bReplenishing = true;
	float ReplenishRate = 0.f;
	float AmountToReplenish = 0.f;

	/*
	 * Speed Buff 
	 */
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float initialBaseSpeed;
	float initialCrouchSpeed;
	UFUNCTION(NetMulticast,Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/*
	 * Jump Buff
	 */
	
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float initialJumpVelocity;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
	 

public:	

		
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGamemode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName Cooldown; // Match Duration Has been Reached. Display Winner and Start CoolDownTimer
}
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGamemode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGamemode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);


	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 20.f;

	float LevelStartingTime = 0.f;

	bool bTeamMatch = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountDownTime = 0.f;

public:
	FORCEINLINE float GetCountDownTime() const { return CountDownTime; }
};

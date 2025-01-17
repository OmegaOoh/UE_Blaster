// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGamemode.h"
#include "TeamsGamemode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamsGamemode : public ABlasterGamemode
{
	GENERATED_BODY()

public:
	ATeamsGamemode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;



protected:
	virtual void HandleMatchHasStarted() override;
};

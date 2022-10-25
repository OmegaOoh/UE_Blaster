// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"


void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayer);
	DOREPLIFETIME(ABlasterGameState, RedTeam_Score);
	DOREPLIFETIME(ABlasterGameState, BlueTeam_Score);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if(TopScoringPlayer.Num() == 0)
	{
		TopScoringPlayer.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayer.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayer.Empty();
		TopScoringPlayer.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ABlasterGameState::RedTeamScores()
{
	++RedTeam_Score;

	ABlasterPlayerController* PController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(PController)
	{
		PController->SetHUDRedTeamScore(RedTeam_Score);
	}
}

void ABlasterGameState::BlueTeamScores()
{
	++BlueTeam_Score;

	ABlasterPlayerController* PController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PController)
	{
		PController->SetHUDBlueTeamScore(BlueTeam_Score);
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	ABlasterPlayerController* PController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PController)
	{
		PController->SetHUDRedTeamScore(RedTeam_Score);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	ABlasterPlayerController* PController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PController)
	{
		PController->SetHUDBlueTeamScore(BlueTeam_Score);
	}
}

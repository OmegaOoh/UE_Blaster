// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Gamemode/BlasterGamemode.h"
#include "Blaster/HUD/Annoucement.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());

	ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);

}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();

	if(HasAuthority())
	{
		if(MatchState == MatchState::InProgress)
		{
			HandleMatchHasStarted();
		}
	}
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{

	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;
	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}

}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}

}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
			FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
			BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountDownText;

	if(bHUDValid)
	{
		if (CountDownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountDownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountDownTime / 60);
		int32 Seconds = CountDownTime - Minutes * 60;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		BlasterHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}

void ABlasterPlayerController::SetHUDAnnoucementCountdown(float CountDownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		if(CountDownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountDownTime / 60);
		int32 Seconds = CountDownTime - Minutes * 60;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountDownText));
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;

	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CoolDownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	/*
	if (HasAuthority())
	{
		BlasterGamemode = BlasterGamemode == nullptr ? Cast<ABlasterGamemode>(UGameplayStatics::GetGameMode(this)) : BlasterGamemode;
		if (BlasterGamemode)
		{
			LevelStartingTime = BlasterGamemode->LevelStartingTime;
			SecondsLeft = FMath::CeilToInt(BlasterGamemode->GetCountDownTime() + LevelStartingTime);
		}
	}
	*/

	if(CountDownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnoucementCountdown(TimeLeft);
		}

		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);
		}
	}

	CountDownInt = SecondsLeft;
}

void ABlasterPlayerController::SetHUDNoWeaponName()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponNameText;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->WeaponNameText->SetText(FText());
	}
}

void ABlasterPlayerController::SetHUDWeaponName(EWeaponType WeaponName)
{
	;
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponNameText;

	
	if (bHUDValid)
	{
		const UEnum* Enumptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EWeaponType"), true);

		FText WeaponTypeText;
		if(Enumptr)
		{
			WeaponTypeText = Enumptr->GetDisplayNameTextByIndex(static_cast<int32>(WeaponName));
			BlasterHUD->CharacterOverlay->WeaponNameText->SetText(WeaponTypeText);
		}
		
	}
}

void ABlasterPlayerController::SetHUDGrenade(int32 Grenade)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadeAmount;
	if (bHUDValid)
	{
		FString GrenadeText = FString::Printf(TEXT("%d"), Grenade);
		BlasterHUD->CharacterOverlay->GrenadeAmount->SetText(FText::FromString(GrenadeText));
	}
	else
	{
		HUDGrenades = Grenade;
	}
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);

				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if (BlasterCharacter && BlasterCharacter->GetCombatComp())
				{
					SetHUDGrenade(BlasterCharacter->GetCombatComp()->GetGrenades());
				}
				else
				{
					SetHUDGrenade(HUDGrenades);
				}
			}
		}
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGamemode* Gamemode = Cast<ABlasterGamemode>(UGameplayStatics::GetGameMode(this));
	if(Gamemode)
	{
		WarmupTime = Gamemode->WarmupTime;
		MatchTime = Gamemode->MatchTime;
		LevelStartingTime = Gamemode->LevelStartingTime;
		MatchState = Gamemode->GetMatchState();
		CoolDownTime = Gamemode->CooldownTime;
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CoolDownTime, LevelStartingTime);

		if (BlasterHUD && MatchState == MatchState::WaitingToStart)
		{
			BlasterHUD->AddAnouncement();
		}
		
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateofMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateofMatch;
	CoolDownTime = Cooldown;
	OnMatchStateSet(MatchState);

	if (BlasterHUD && MatchState == MatchState::WaitingToStart && !HasAuthority())
	{
		BlasterHUD->AddAnouncement();
	}
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
		MatchState = State;
		if (MatchState == MatchState::InProgress)
		{
			HandleMatchHasStarted();
		}
		else if (MatchState == MatchState::Cooldown)
		{
			HandleCoolDown();
		}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCoolDown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD)
	{
		if(BlasterHUD && !BlasterHUD->CharacterOverlay)
		{
			BlasterHUD->AddCharacterOverlay();
		}

		if (BlasterHUD->Announcement)
		{
				BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		
	}
}

void ABlasterPlayerController::HandleCoolDown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;


	if (BlasterHUD)
	{
		if (BlasterHUD->Announcement == nullptr)
		{
			BlasterHUD->AddAnouncement();
		}

		BlasterHUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid = BlasterHUD->Announcement && 
			BlasterHUD->Announcement->AnnoucementText && 
			BlasterHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnoucementText("New Match Start in:");
			BlasterHUD->Announcement->AnnoucementText->SetText(FText::FromString(AnnoucementText));

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();

			if(BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayer = BlasterGameState->TopScoringPlayer;
				FString InfoTextString;
				if(TopPlayer.Num() == 0)
				{
					InfoTextString = FString("There is no winner.");
				}
				else if(TopPlayer.Num() == 1 && TopPlayer[0] == BlasterPlayerState)
				{
					InfoTextString = FString::Printf(TEXT("You are The Winner!"));
		
				}
				else if (TopPlayer.Num() == 1 )
				{
					InfoTextString = FString::Printf(TEXT("Winner : \n %s"), *TopPlayer[0]->GetPlayerName());
				}
				else if (TopPlayer.Num() > 1)
				{
					InfoTextString = FString("Player tied for the win:\n");
					for (auto TiedPlayer : TopPlayer)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
				
				
			}
			else if (BlasterGameState)
			{
				FString Error("Error (BlasterGameState is Valid") ;
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(Error));
			}
			else if (BlasterPlayerState)
			{
				FString Error("Error (BlasterPlayerState is Valid");
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(Error));
			}
		}
	}
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	
	if(BlasterCharacter && BlasterCharacter->GetCombatComp())
	{
		BlasterCharacter->bDisableGameplay = true;
		BlasterCharacter->GetCombatComp()->FireButtonPressed(false);
	}

}


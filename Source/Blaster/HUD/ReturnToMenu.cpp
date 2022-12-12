// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMenu.h"

#include "../../../Plugins/MultiplayerSessions/Source/MultiplayerSessions/Public/MultiplayerSessionsSubsystem.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"

void UReturnToMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if(World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if(PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	if( ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMenu::ReturnButtonClicked);
	}
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMenu::OnDestroySession);
		}

	}
}

void UReturnToMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if (ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMenu::ReturnButtonClicked);
	}
	if(MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMenu::OnDestroySession);
	}

}

bool UReturnToMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if(ReturnButton)
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMenu::ReturnButtonClicked);
	}

	return true;
}

void UReturnToMenu::OnDestroySession(bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}


	UWorld* World = GetWorld();
	if(World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if(GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if(PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UReturnToMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if(FirstPlayerController)
		{
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FirstPlayerController->GetPawn());
			if(BlasterCharacter)
			{
				BlasterCharacter->ServerLeaveGame();
				BlasterCharacter->OnLeftGame.AddDynamic(this, &UReturnToMenu::OnPlayerLeftGame);
			}
			else
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}

void UReturnToMenu::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMenu::OnDestroySession);
	}
}
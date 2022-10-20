#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Annoucement.h"
#include "KillFeedWidget.h"
#include "Blaster/Gamemode/BlasterGamemode.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"


void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && AnnoucementClass)
	{
		Announcement = CreateWidget<UAnnoucement>(PlayerController, AnnoucementClass);
		Announcement->AddToViewport();
	}
	
}
/*
 * Kill Feed
 */
void ABlasterHUD::AddKillFeedWidget(FString Killer,FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if(OwningPlayer && KillFeedClass)
	{
		UKillFeedWidget* KillFeedWidget = CreateWidget<UKillFeedWidget>(OwningPlayer, KillFeedClass);
		if(KillFeedWidget)
		{
			KillFeedWidget->SetKillFeedText(Killer, Victim);
			KillFeedWidget->AddToViewport();

			for(UKillFeedWidget* Msg: KillMessages)
			{
				if(Msg && Msg->FeedBlock)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->FeedBlock);
					if(CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(
							CanvasSlot->GetPosition().X,
							Position.Y - CanvasSlot->GetSize().Y
						);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}



			KillMessages.Add(KillFeedWidget);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("KillFeedTimerFinished"), KillFeedWidget);
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,
				ElimMsgDelegate,
				KillFeedTime,
				false
			);
		}
	}
}

void ABlasterHUD::KillFeedTimerFinished(UKillFeedWidget* MsgToRemove)
{
	if(MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter,Spread,HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter,Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter,Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread( 0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter,Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread( 0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}



void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint
	(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture
	(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}



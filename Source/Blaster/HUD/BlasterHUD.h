// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;

	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;

	//CharacterOverlay

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	void AddCharacterOverlay();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	//Announcements

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<class UUserWidget> AnnoucementClass;

	UPROPERTY()
	class UAnnoucement* Announcement;

	void AddAnouncement();

	void AddKillFeedWidget(FString Killer, FString Victim);

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY()
	class APlayerController* OwningPlayer;

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere, Category = "KillFeed")
	TSubclassOf<class UKillFeedWidget> KillFeedClass;

	UPROPERTY(EditAnywhere,Category = "KillFeed")
	float KillFeedTime = 2.5f;

	UFUNCTION()
	void KillFeedTimerFinished(UKillFeedWidget* MsgToRemove);

	UPROPERTY()
	TArray<UKillFeedWidget*> KillMessages;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};

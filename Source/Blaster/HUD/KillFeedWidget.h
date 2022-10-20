// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KillFeedWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UKillFeedWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetKillFeedText(FString KillerName, FString VictimName);

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* FeedBlock;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* FeedText;

};

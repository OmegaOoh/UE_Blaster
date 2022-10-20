// Fill out your copyright notice in the Description page of Project Settings.


#include "KillFeedWidget.h"
#include "Components/TextBlock.h"


void UKillFeedWidget::SetKillFeedText(FString KillerName, FString VictimName)
{
	FString KillFeedText = FString::Printf(TEXT("%s elimmed %s"), *KillerName, *VictimName);
	if(FeedText)
	{
		FeedText->SetText(FText::FromString(KillFeedText));
	}
}

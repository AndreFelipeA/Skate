// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"

#include "ScoreWidget.h"
#include "OptionsMenu.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	if(ScoreWidgetClass)
	{
		ScoreWidget = CreateWidget<UScoreWidget>(GetWorld(), ScoreWidgetClass);
		ScoreWidget->AddToViewport();
		ScoreWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if(OptionsMenuClass)
	{
		OptionsMenu = CreateWidget<UOptionsMenu>(GetWorld(), OptionsMenuClass);
		OptionsMenu->AddToViewport();
		OptionsMenu->SetVisibility(ESlateVisibility::Hidden);
	}
}

void APlayerHUD::ShowOptionsMenu(bool bShow)
{
	APlayerController* Controller = Cast<APlayerController>(this->Owner);
	if(bShow)
	{
		if(Controller)
		{
			
			Controller->SetInputMode(FInputModeGameAndUI());
			Controller->bShowMouseCursor = true;
		}
		OptionsMenu->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		if(Controller)
		{
			Controller->SetInputMode(FInputModeGameOnly());
			Controller->bShowMouseCursor = false;
		}
		OptionsMenu->SetVisibility(ESlateVisibility::Hidden);
	}
}

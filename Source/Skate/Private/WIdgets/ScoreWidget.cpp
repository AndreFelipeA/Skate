// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreWidget.h"

#include "Skate/SkateCharacter.h"

float UScoreWidget::GetScore()
{
	FString TextString = ScoreText->GetText().ToString();
	return FCString::Atof(*TextString);
}

void UScoreWidget::RefreshScore(float ScoreF) const
{
	if(ScoreText)
	{
		FText Text = FText::AsNumber(ScoreF);
		ScoreText->SetText(Text);
	}
}

void UScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Character = Cast<ASkateCharacter>(GetOwningPlayerPawn());

	if(Character)
	{
		Character->OnScoreChangedDelegate.AddUObject(this, &UScoreWidget::RefreshScore);
	}
}

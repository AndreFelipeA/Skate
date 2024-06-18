// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ScoreWidget.generated.h"


class ASkateCharacter;
/**
 * 
 */
UCLASS()
class UScoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreText;

	UPROPERTY()
	ASkateCharacter* Character;

	float GetScore();

	void RefreshScore(float ScoreF) const;

protected:
	virtual void NativeOnInitialized() override;
};

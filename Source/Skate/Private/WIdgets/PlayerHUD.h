// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

class UOptionsMenu;
class UScoreWidget;
/**
 * 
 */
UCLASS()
class APlayerHUD : public AHUD
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UScoreWidget> ScoreWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UOptionsMenu> OptionsMenuClass;

	void ShowOptionsMenu(bool bShow);
	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY()
	UScoreWidget* ScoreWidget;

	UPROPERTY()
	UOptionsMenu* OptionsMenu;


};

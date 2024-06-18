// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "OptionsMenu.generated.h"

/**
 * 
 */
UCLASS()
class UOptionsMenu : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Continue;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Menu;

	UFUNCTION()
	void CloseOptionsMenu();
	
	virtual void NativeConstruct() override;
};

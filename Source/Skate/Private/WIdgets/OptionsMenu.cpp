// Fill out your copyright notice in the Description page of Project Settings.


#include "OptionsMenu.h"

#include "Kismet/GameplayStatics.h"
#include "Skate/SkateCharacter.h"



void UOptionsMenu::NativeConstruct()
{
	if(Continue)
	{
		Continue->OnClicked.AddDynamic(this, &UOptionsMenu::CloseOptionsMenu);
	}
}
void UOptionsMenu::CloseOptionsMenu()
{
	APlayerController* Controller = Cast<APlayerController>(this->GetOwningPlayer());
	if(Controller)
	{
		
		Controller->SetInputMode(FInputModeGameOnly());
		Controller->bShowMouseCursor = false;
		ASkateCharacter* Character = Cast<ASkateCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
		if(Character)
		{
			Character->OptionsMenu = false;
		}
	}
	this->SetVisibility(ESlateVisibility::Hidden);
	
}



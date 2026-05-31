// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionMainMenuWidget.h"

#include "DemolitionPlayerController.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UDemolitionMainMenuWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (Button_DemoLevels)
		Button_DemoLevels->OnClicked.AddDynamic(this, &UDemolitionMainMenuWidget::HandleDemoLevelsClicked);

	if (Button_SweepSelect)
		Button_SweepSelect->OnClicked.AddDynamic(this, &UDemolitionMainMenuWidget::HandleSweepSelectClicked);
	
	if (Button_Quit)
		Button_Quit->OnClicked.AddDynamic(this, &UDemolitionMainMenuWidget::HandleQuitClicked);
}

void UDemolitionMainMenuWidget::HandleDemoLevelsClicked() {
	if (auto* PC = Cast<ADemolitionPlayerController>(GetOwningPlayer()))
		PC->ShowLevelSelect();
}

void UDemolitionMainMenuWidget::HandleSweepSelectClicked() {
	if (auto* PC = Cast<ADemolitionPlayerController>(GetOwningPlayer()))
		PC->ShowSweepSelect();
}

void UDemolitionMainMenuWidget::HandleQuitClicked() {
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

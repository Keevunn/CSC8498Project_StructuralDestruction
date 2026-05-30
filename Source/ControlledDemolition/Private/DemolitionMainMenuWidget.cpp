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
}

void UDemolitionMainMenuWidget::HandleDemoLevelsClicked() {
	if (auto* PC = Cast<ADemolitionPlayerController>(GetOwningPlayer()))
		PC->ShowLevelSelect();
}

void UDemolitionMainMenuWidget::HandleSweepSelectClicked() {
	if (auto* PC = Cast<ADemolitionPlayerController>(GetOwningPlayer()))
		PC->ShowSweepSelect();
}
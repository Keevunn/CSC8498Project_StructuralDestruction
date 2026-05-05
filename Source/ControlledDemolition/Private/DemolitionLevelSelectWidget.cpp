// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionLevelSelectWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UDemolitionLevelSelectWidget::RefreshLevelList() {
	return;
}

void UDemolitionLevelSelectWidget::NativeConstruct() {
	Super::NativeConstruct();
	
	if (Button_LevelOne)
		Button_LevelOne->OnClicked.AddDynamic(this, &UDemolitionLevelSelectWidget::HandleLevelOneClicked);
	
	if (Button_LevelTwo)
		Button_LevelTwo->OnClicked.AddDynamic(this, &UDemolitionLevelSelectWidget::HandleLevelTwoClicked);
	
	if (Button_LevelThree)
		Button_LevelThree->OnClicked.AddDynamic(this, &UDemolitionLevelSelectWidget::HandleLevelThreeClicked);
}

// TODO ALL JOB LEVEL NAMES HERE ARE PLACEHOLDERS

void UDemolitionLevelSelectWidget::HandleLevelOneClicked() {
	OpenJobLevel(TEXT("Level_SimpleTower"));
}

void UDemolitionLevelSelectWidget::HandleLevelTwoClicked() {
	OpenJobLevel(TEXT("Job02"));
}

void UDemolitionLevelSelectWidget::HandleLevelThreeClicked() {
	OpenJobLevel(TEXT("Job03"));
}

void UDemolitionLevelSelectWidget::OpenJobLevel(const FName LevelName) {
	UGameplayStatics::OpenLevel(this, LevelName);
}

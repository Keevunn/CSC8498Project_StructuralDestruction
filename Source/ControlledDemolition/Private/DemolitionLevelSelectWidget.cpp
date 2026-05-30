// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionLevelSelectWidget.h"

#include "DemolitionPlayerController.h"
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
	
	if (Button_Back)
		Button_Back->OnClicked.AddDynamic(this, &UDemolitionLevelSelectWidget::HandleBackClicked);
}

// TODO JOB LEVEL NAMES HERE ARE PLACEHOLDERS

void UDemolitionLevelSelectWidget::HandleLevelOneClicked() {
	OpenJobLevel(TEXT("Level_DemoCONN"));
}

void UDemolitionLevelSelectWidget::HandleLevelTwoClicked() {
	OpenJobLevel(TEXT("Level_DemoPHYS"));
}

void UDemolitionLevelSelectWidget::HandleLevelThreeClicked() {
	OpenJobLevel(TEXT("Level_DemoLOAD"));
}

void UDemolitionLevelSelectWidget::HandleBackClicked() {
	if (auto* PC = Cast<ADemolitionPlayerController>(GetOwningPlayer()))
		PC->ShowMainMenu();
}

void UDemolitionLevelSelectWidget::OpenJobLevel(const FName LevelName) {
	UGameplayStatics::OpenLevel(this, LevelName);
}

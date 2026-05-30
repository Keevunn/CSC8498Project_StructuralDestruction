// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionLevelSelectWidget.h"

#include "DemolitionPlayerController.h"
#include "JobManagerSubsystem.h"
#include "JobTypes.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

class UJobManagerSubsystem;

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
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UJobManagerSubsystem* JobManager = GI->GetSubsystem<UJobManagerSubsystem>();
	if (!JobManager) return;
	
	FJobDefinition Job;
	Job.LevelName = LevelName;
	Job.MaxCharges = 5;
	
	if (LevelName == TEXT("Level_DemoCONN"))		Job.JobName = TEXT("CONN Demo");
	else if (LevelName == TEXT("Level_DemoPHYS"))	Job.JobName = TEXT("PHYS Demo");
	else if (LevelName == TEXT("Level_DemoLOAD"))	Job.JobName = TEXT("LOAD Demo");
	else											Job.JobName = TEXT("Demo Job");
	
	JobManager->StartJob(Job);
}

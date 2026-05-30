// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionMainMenuWidget.h"

#include "DemolitionPlayerController.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UDemolitionMainMenuWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (Button_DemoLevels)
		Button_DemoLevels->OnClicked.AddDynamic(this, &UDemolitionMainMenuWidget::HandleDemoLevelsClicked);

	if (Button_Benchmark)
		Button_Benchmark->OnClicked.AddDynamic(this, &UDemolitionMainMenuWidget::HandleBenchmarkClicked);
}

void UDemolitionMainMenuWidget::HandleDemoLevelsClicked() {
	if (auto* PC = Cast<ADemolitionPlayerController>(GetOwningPlayer()))
		PC->ShowLevelSelect();
}

void UDemolitionMainMenuWidget::HandleBenchmarkClicked() {
	UGameplayStatics::OpenLevel(this, BenchmarkLevelName);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionBenchmarkRunnerWidget.h"

#include "DemolitionBenchmarkSubsystem.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UDemolitionBenchmarkRunnerWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (Text_SweepStatus)
		Text_SweepStatus->SetText(FText::FromString(TEXT("Running benchmark sweeps...")));
	if (Text_RunStatus)
		Text_RunStatus->SetText(FText::FromString(TEXT("Run:")));
	if (Text_ScenarioName)
		Text_ScenarioName->SetText(FText::FromString(TEXT("Scenario:")));
	if (Text_ModelName)
		Text_ModelName->SetText(FText::FromString(TEXT("Model:")));

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UDemolitionBenchmarkSubsystem* BenchmarkSystem = GI->GetSubsystem<UDemolitionBenchmarkSubsystem>();
	if (!BenchmarkSystem) return;

	BenchmarkSystem->OnSweepStarted.AddDynamic(this, &UDemolitionBenchmarkRunnerWidget::HandleSweepStarted);
	BenchmarkSystem->OnRunStarted.AddDynamic(this, &UDemolitionBenchmarkRunnerWidget::HandleRunStarted);
	BenchmarkSystem->OnAllSweepsComplete.AddDynamic(this, &UDemolitionBenchmarkRunnerWidget::HandleSweepsComplete);
	BenchmarkSystem->RunAllSweeps();
}

void UDemolitionBenchmarkRunnerWidget::NativeDestruct() {
	if (UGameInstance* GI = GetGameInstance())
		if (UDemolitionBenchmarkSubsystem* BenchmarkSystem = GI->GetSubsystem<UDemolitionBenchmarkSubsystem>()) {
			BenchmarkSystem->OnSweepStarted.RemoveDynamic(this, &UDemolitionBenchmarkRunnerWidget::HandleSweepStarted);
			BenchmarkSystem->OnRunStarted.RemoveDynamic(this, &UDemolitionBenchmarkRunnerWidget::HandleRunStarted);
			BenchmarkSystem->OnAllSweepsComplete.RemoveDynamic(this, &UDemolitionBenchmarkRunnerWidget::HandleSweepsComplete);
		}

	Super::NativeDestruct();
}

void UDemolitionBenchmarkRunnerWidget::HandleSweepStarted(int32 SweepNumber, int32 TotalSweeps) {
	if (!Text_SweepStatus) return;
	Text_SweepStatus->SetText(FText::FromString(FString::Printf(TEXT("Sweep: %d / %d"), SweepNumber, TotalSweeps)));
	
	if (Text_ScenarioName) {
		FString ScenarioName;
		switch (SweepNumber) {
			case 1: ScenarioName = "AnchorRemoval"; break;
			case 2: ScenarioName = "SupportRemoval"; break;
			case 3: ScenarioName = "LoadRedistribution"; break;
			case 4: ScenarioName = "ProtectedPreservation"; break;
			default: ScenarioName = "";
		}
		
		Text_ScenarioName->SetText(FText::FromString(FString::Printf(TEXT("Scenario: %s"), *ScenarioName)));
	}
}

void UDemolitionBenchmarkRunnerWidget::HandleRunStarted(int32 RunNumber, int32 TotalRuns) {
	if (!Text_RunStatus) return;
	Text_RunStatus->SetText(FText::FromString(FString::Printf(TEXT("Run: %d / %d"), RunNumber, TotalRuns)));
	
	if (Text_ModelName) {
		int32 RunsPerModel = TotalRuns / 3; // Always int as long as only 3 models
		FString ModelName = RunNumber <= RunsPerModel ? "PHYS" : RunNumber <= 2 * RunsPerModel ? "CONN" : "LOAD";
		Text_ModelName->SetText(FText::FromString(FString::Printf(TEXT("Model: %s"), *ModelName)));
	}
}

void UDemolitionBenchmarkRunnerWidget::HandleSweepsComplete() {
	if (Text_SweepStatus)
		Text_SweepStatus->SetText(FText::FromString(TEXT("Complete. Returning to menu...")));

	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemolitionBenchmarkRunnerWidget.generated.h"

enum class EBenchmarkScenario : uint8;
class UTextBlock;

UCLASS()
class CONTROLLEDDEMOLITION_API UDemolitionBenchmarkRunnerWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void HandleSweepStarted(int32 SweepNumber, int32 TotalSweeps, EBenchmarkScenario Scenario);
	
	UFUNCTION()
	void HandleRunStarted(int32 RunNumber, int32 TotalRuns);
	
	UFUNCTION()
	void HandleSweepsComplete();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SweepStatus;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RunStatus;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ScenarioName;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ModelName;
	
	UPROPERTY(EditDefaultsOnly, Category="Maps")
	FName MainMenuLevelName = TEXT("Level_MainMenu");
};

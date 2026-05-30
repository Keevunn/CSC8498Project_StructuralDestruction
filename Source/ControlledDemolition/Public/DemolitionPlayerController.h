// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructureSpawner.h"
#include "GameFramework/PlayerController.h"
#include "DemolitionPlayerController.generated.h"

class UDemolitionSweepSelectWidget;
class UInputAction;
class UDemolitionMainMenuWidget;
class UDemolitionBenchmarkRunnerWidget;
class UDemolitionLevelSelectWidget;
class UDemolitionResultWidget;
class UDemolitionHUDWidget;

UCLASS()
class CONTROLLEDDEMOLITION_API ADemolitionPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
	void ShowMainMenu();
	void ShowGameplayHUD();
	void ShowResultScreen(bool bWonLevel, int32 FinalScore, int32 MoneyEarned, float DestructionRatio);
	void ShowLevelSelect();
	void ShowBenchmarkRunner();
	void ShowSweepSelect();
	
	void UpdateHUDCharges(int32 ChargesUsed, int32 MaxCharges);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDemolitionHUDWidget> HUDWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDemolitionResultWidget> ResultWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDemolitionLevelSelectWidget> LevelSelectWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDemolitionMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDemolitionBenchmarkRunnerWidget> BenchmarkRunnerWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UDemolitionSweepSelectWidget> SweepSelectWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UDemolitionHUDWidget> HUDWidget;
	
	UPROPERTY()
	TObjectPtr<UDemolitionResultWidget> ResultWidget;
	
	UPROPERTY()
	TObjectPtr<UDemolitionLevelSelectWidget> LevelSelectWidget;
	
	UPROPERTY()
	TObjectPtr<UDemolitionMainMenuWidget> MainMenuWidget;

	UPROPERTY()
	TObjectPtr<UDemolitionBenchmarkRunnerWidget> BenchmarkRunnerWidget;
	
	UPROPERTY()
	TObjectPtr<UDemolitionSweepSelectWidget> SweepSelectWidget;
	
	// Input for debug
	virtual void SetupInputComponent() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleDrawDebugAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleVerboseLogsAction;
	
	void HandleToggleDrawDebug();
	void HandleToggleVerboseLogs();
	
private:
	void HideAllWidgets();
	void SetGameInput();
	void SetUIInput();
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	bool bAutoShowGameplayHUD = true;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionPlayerController.h"

#include "DemolitionBenchmarkRunnerWidget.h"
#include "DemolitionGameState.h"
#include "DemolitionHUDWidget.h"
#include "DemolitionLevelSelectWidget.h"
#include "DemolitionMainMenuWidget.h"
#include "DemolitionResultWidget.h"
#include "Blueprint/UserWidget.h"

void ADemolitionPlayerController::BeginPlay() {
	Super::BeginPlay();
	
	if (bAutoShowGameplayHUD) ShowGameplayHUD(); 
	
	if (ADemolitionGameState* GameState = GetWorld()->GetGameState<ADemolitionGameState>()) {
		GameState->OnChargePlaced.AddUObject(this, &ADemolitionPlayerController::UpdateHUDCharges);
		UpdateHUDCharges(GameState->GetChargesUsed(), GameState->GetMaxCharges());
		
		GameState->OnJobCompleted.AddUObject(this, &ADemolitionPlayerController::ShowResultScreen);
	}
}

void ADemolitionPlayerController::ShowMainMenu() {
	HideAllWidgets();

	if (!MainMenuWidget && MainMenuWidgetClass)
		MainMenuWidget = CreateWidget<UDemolitionMainMenuWidget>(this, MainMenuWidgetClass);

	if (!MainMenuWidget) return;

	MainMenuWidget->AddToViewport();
	SetUIInput(MainMenuWidget);
}

void ADemolitionPlayerController::ShowGameplayHUD() {
	HideAllWidgets();
	
	if (!HUDWidget && HUDWidgetClass)
		HUDWidget = CreateWidget<UDemolitionHUDWidget>(this, HUDWidgetClass);
	
	if (HUDWidget)
		HUDWidget->AddToViewport();
	
	SetGameInput();
}

void ADemolitionPlayerController::ShowResultScreen(bool bWonLevel, int32 FinalScore, int32 MoneyEarned,
	float DestructionRatio) {
	HideAllWidgets();
	
	if (!ResultWidget && ResultWidgetClass)
		ResultWidget = CreateWidget<UDemolitionResultWidget>(this, ResultWidgetClass);
	
	if (!ResultWidget) return;
	
	ResultWidget->SetResultData(bWonLevel, FinalScore, MoneyEarned, DestructionRatio);
	ResultWidget->AddToViewport();
	SetUIInput(ResultWidget);
}

void ADemolitionPlayerController::ShowLevelSelect() {
	HideAllWidgets();
	
	if (!LevelSelectWidget && LevelSelectWidgetClass)
		LevelSelectWidget = CreateWidget<UDemolitionLevelSelectWidget>(this, LevelSelectWidgetClass);
	
	if (!LevelSelectWidget) return;
	
	LevelSelectWidget->AddToViewport();
	SetUIInput(LevelSelectWidget);
}

void ADemolitionPlayerController::ShowBenchmarkRunner() {
	HideAllWidgets();

	if (!BenchmarkRunnerWidget && BenchmarkRunnerWidgetClass)
		BenchmarkRunnerWidget = CreateWidget<UDemolitionBenchmarkRunnerWidget>(this, BenchmarkRunnerWidgetClass);

	if (!BenchmarkRunnerWidget) return;

	BenchmarkRunnerWidget->AddToViewport();
	SetUIInput(BenchmarkRunnerWidget);
}

void ADemolitionPlayerController::UpdateHUDCharges(int32 ChargesUsed, int32 MaxCharges) {
	if (HUDWidget)
		HUDWidget->SetChargesData(ChargesUsed, MaxCharges);
}

void ADemolitionPlayerController::HideAllWidgets() {
	if (HUDWidget) HUDWidget->RemoveFromParent();
	
	if (ResultWidget) ResultWidget->RemoveFromParent();
	
	if (LevelSelectWidget) LevelSelectWidget->RemoveFromParent();
	
	if (MainMenuWidget) MainMenuWidget->RemoveFromParent();
	
	if (BenchmarkRunnerWidget) BenchmarkRunnerWidget->RemoveFromParent();
}

void ADemolitionPlayerController::SetGameInput() {
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void ADemolitionPlayerController::SetUIInput(UUserWidget* FocusWidget) {
	FInputModeUIOnly InputMode;
	
	if (FocusWidget)
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
	
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

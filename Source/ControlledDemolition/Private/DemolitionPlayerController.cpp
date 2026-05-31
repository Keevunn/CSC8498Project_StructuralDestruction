// Fill out your copyright notice in the Description page of Project Settings.


#include "DemolitionPlayerController.h"

#include "DemolitionBenchmarkRunnerWidget.h"
#include "DemolitionDebug.h"
#include "DemolitionGameState.h"
#include "DemolitionHUDWidget.h"
#include "DemolitionLevelSelectWidget.h"
#include "DemolitionMainMenuWidget.h"
#include "DemolitionResultWidget.h"
#include "DemolitionSweepSelectWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"

void ADemolitionPlayerController::BeginPlay() {
	Super::BeginPlay();
	
	if (GlobalMappingContext)
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			Subsystem->AddMappingContext(GlobalMappingContext, 0);
	
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
	SetUIInput();
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
	SetUIInput();
}

void ADemolitionPlayerController::ShowLevelSelect() {
	HideAllWidgets();
	
	if (!LevelSelectWidget && LevelSelectWidgetClass)
		LevelSelectWidget = CreateWidget<UDemolitionLevelSelectWidget>(this, LevelSelectWidgetClass);
	
	if (!LevelSelectWidget) return;
	
	LevelSelectWidget->AddToViewport();
	SetUIInput();
}

void ADemolitionPlayerController::ShowBenchmarkRunner() {
	HideAllWidgets();

	if (!BenchmarkRunnerWidget && BenchmarkRunnerWidgetClass)
		BenchmarkRunnerWidget = CreateWidget<UDemolitionBenchmarkRunnerWidget>(this, BenchmarkRunnerWidgetClass);

	if (!BenchmarkRunnerWidget) return;

	BenchmarkRunnerWidget->AddToViewport();
	SetUIInput();
}

void ADemolitionPlayerController::ShowSweepSelect() {
	HideAllWidgets();
	
	if (!SweepSelectWidget && SweepSelectWidgetClass)
		SweepSelectWidget = CreateWidget<UDemolitionSweepSelectWidget>(this, SweepSelectWidgetClass);
	
	if (!SweepSelectWidget) return;
	
	SweepSelectWidget->AddToViewport();
	SetUIInput();
}

void ADemolitionPlayerController::UpdateHUDCharges(int32 ChargesUsed, int32 MaxCharges) {
	if (HUDWidget)
		HUDWidget->SetChargesData(ChargesUsed, MaxCharges);
}

void ADemolitionPlayerController::SetGameplayInputEnabled(bool bEnabled) {
	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem || !GameplayMappingContext) return;

	if (bEnabled)
		Subsystem->AddMappingContext(GameplayMappingContext, 1);
	else
		Subsystem->RemoveMappingContext(GameplayMappingContext);
}

void ADemolitionPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();
	
	if (auto* EIC = Cast<UEnhancedInputComponent>(InputComponent)) {
		if (ToggleDrawDebugAction)
			EIC->BindAction(ToggleDrawDebugAction, ETriggerEvent::Started, this, &ADemolitionPlayerController::HandleToggleDrawDebug);
		if (ToggleVerboseLogsAction)
			EIC->BindAction(ToggleVerboseLogsAction, ETriggerEvent::Started, this, &ADemolitionPlayerController::HandleToggleVerboseLogs);
		if (QuitGameAction)
			EIC->BindAction(QuitGameAction, ETriggerEvent::Started, this, &ADemolitionPlayerController::HandleQuitGame);
	}
}

void ADemolitionPlayerController::HandleToggleDrawDebug() {
	DemolitionDebug::ToggleDrawDebug();
}

void ADemolitionPlayerController::HandleToggleVerboseLogs() {
	DemolitionDebug::ToggleVerboseLogs();
}

void ADemolitionPlayerController::HandleQuitGame() {
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void ADemolitionPlayerController::HideAllWidgets() {
	if (HUDWidget) HUDWidget->RemoveFromParent();
	
	if (ResultWidget) ResultWidget->RemoveFromParent();
	
	if (LevelSelectWidget) LevelSelectWidget->RemoveFromParent();
	
	if (MainMenuWidget) MainMenuWidget->RemoveFromParent();
	
	if (BenchmarkRunnerWidget) BenchmarkRunnerWidget->RemoveFromParent();
	
	if (SweepSelectWidget) SweepSelectWidget->RemoveFromParent();
}

void ADemolitionPlayerController::SetGameInput() {
	SetGameplayInputEnabled(true);
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void ADemolitionPlayerController::SetUIInput() {
	SetGameplayInputEnabled(false);
	FInputModeGameAndUI InputMode;
	
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DemolitionPlayerController.generated.h"

class UDemolitionLevelSelectWidget;
class UDemolitionResultWidget;
class UDemolitionHUDWidget;

UCLASS()
class CONTROLLEDDEMOLITION_API ADemolitionPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
	void ShowGameplayHUD();
	void ShowResultScreen(bool bWonLevel, int32 FinalScore, int32 MoneyEarned, float DestructionRatio);
	void ShowLevelSelect();
	
	void UpdateHUDCharges(int32 ChargesUsed, int32 MaxCharges);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDemolitionHUDWidget> HUDWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDemolitionResultWidget> ResultWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDemolitionLevelSelectWidget> LevelSelectWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UDemolitionHUDWidget> HUDWidget;
	
	UPROPERTY()
	TObjectPtr<UDemolitionResultWidget> ResultWidget;
	
	UPROPERTY()
	TObjectPtr<UDemolitionLevelSelectWidget> LevelSelectWidget;
	
private:
	void HideAllWidgets();
	void SetGameInput();
	void SetUIInput(UUserWidget* FocusWidget);
};

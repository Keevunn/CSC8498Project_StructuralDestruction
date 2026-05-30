// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BenchmarkTypes.h"
#include "Blueprint/UserWidget.h"
#include "DemolitionSweepSelectWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class CONTROLLEDDEMOLITION_API UDemolitionSweepSelectWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	UFUNCTION() void HandleProfileDemoClicked();
	UFUNCTION() void HandleProfileFullClicked();

	UFUNCTION() void HandleAnchorClicked();
	UFUNCTION() void HandleSupportClicked();
	UFUNCTION() void HandleLoadClicked();
	UFUNCTION() void HandleProtectedClicked();
	UFUNCTION() void HandleAllClicked();

	UFUNCTION() void HandleBackClicked();

	void UpdateProfileLabel();
	void StartSingle(EBenchmarkScenario Scenario);
	void StartAll();

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_ProfileDemo;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_ProfileFull;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Anchor;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Support;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Load;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Protected;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_All;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Back;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Profile;

	UPROPERTY(EditDefaultsOnly, Category="Maps")
	FName BenchmarkLevelName = TEXT("Level_Benchmark");

	EBenchmarkProfile SelectedProfile = EBenchmarkProfile::Demo;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemolitionMainMenuWidget.generated.h"


class UButton;

UCLASS()
class CONTROLLEDDEMOLITION_API UDemolitionMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void HandleDemoLevelsClicked();
	
	UFUNCTION()
	void HandleBenchmarkClicked();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_DemoLevels;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Benchmark;
	
	UPROPERTY(EditDefaultsOnly, Category="Maps")
	FName BenchmarkLevelName = TEXT("Level_Benchmark");
};

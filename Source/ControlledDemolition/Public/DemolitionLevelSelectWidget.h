// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemolitionLevelSelectWidget.generated.h"


class UButton;

UCLASS()
class CONTROLLEDDEMOLITION_API UDemolitionLevelSelectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void RefreshLevelList();
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void HandleLevelOneClicked();
	
	UFUNCTION()
	void HandleLevelTwoClicked();
	
	UFUNCTION()
	void HandleLevelThreeClicked();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_LevelOne;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_LevelTwo;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_LevelThree;
	
private:
	void OpenJobLevel(FName LevelName);
};
